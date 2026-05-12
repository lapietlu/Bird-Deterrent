// ============================================================
//  Bird Deterrent – WiFi AP Web Server
//  Board: Arduino Giga R1 WiFi
//  Libs:  WiFi (built-in), ArduinoJson, Arduino_USBHostMbed5,
//         Arduino_AdvancedAnalog 
// ============================================================

// Last updated by Luke Pietluck, 5/1/2026
// Added explicatory comments to key features

// Keywords:
// UPDATE: Temporary values, update before final version
// CAUTION: Coded behavior that isn't complete/has potential problems
// TODO: Areas/behavior that has yet to be complete

#include <SPI.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Arduino_USBHostMbed5.h>      // USB host stack
#include <Arduino_AdvancedAnalog.h>    // DAC + WavReader
#include "mbed.h"
#include "mbed_mktime.h"
#include "arduino_secrets.h"           // SECRET_SSID / SECRET_PASS

// Struct defined early due to compilation errors
struct HttpRequest {
  String method;        // "GET" / "POST"
  String path;          // "/files"  (no query string)
  String query;         // "t=123"
  String body;          // small POST bodies only (not uploads)
  String contentType;   // "multipart/form-data; boundary=..."
  long   contentLength = 0;
};

// Prototype required early because Arduino is being picky on struct types
HttpRequest parseRequest(WiFiClient& client);

// Device identity ─────────────────────────────────────────
const String SERIAL_NUM = "BD-2402-7A91C2";
// Version five adds scheduled audio playback via DAC
const String FIRMWARE   = "v0.5.0";

// Network ─────────────────────────────────────────────────
// These are stored in the secrets file
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
WiFiServer server(80);

// USB / Filesystem ────────────────────────────────────────
// Initialize library objects
USBHostMSD  msd;
mbed::FATFileSystem fs("usb");

bool usbMounted = false;

// Config file paths ────────────────────────────────────────
// Write to .tmp first, then rename, so a power failure mid-write
// never corrupts the existing config.json.
#define CONFIG_PATH     "/usb/config.json"
#define CONFIG_TMP_PATH "/usb/config.tmp"

// RTC helpers ─────────────────────────────────────────────
void rtcSetDefault() {
  // Default compile-time clock so the device isn't epoch-zero
  tm t = {};
  t.tm_year  = 126;   // 2026 - 1900
  t.tm_mon   = 0;     // January (0-indexed)
  t.tm_mday  = 1;
  t.tm_hour  = 0;
  t.tm_min   = 0;
  t.tm_sec   = 0;
  set_time(mktime(&t));
}

// Grabbing device time from connected device
String getLocaltime() {
  char buf[32];
  tm t;
  // Converts a given time in seconds since epoch into calendar time for display
  _rtc_localtime(time(NULL), &t, RTC_4_YEAR_LEAP_YEAR_SUPPORT);
  // Converts calendarized time into string var
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &t);
  return String(buf);
}

// USB mount ───────────────────────────────────────────────
// Returns true if the drive mounted successfully.
// Call once after WiFi AP is up; retries a few times to give
// the drive time to spin up.
// Currently set to five seconds via 5 retries of 1000 ms each
// (after testing takes ~3s for response)
bool mountUSB(uint8_t retries = 5, uint16_t delayMs = 1000) {
  for (uint8_t i = 0; i < retries; i++) {
    if (msd.connect()) {
      int err = fs.mount(&msd);
      if (err == 0) {
        Serial.println("[USB] Mounted at /usb");
        return true;
      }
      Serial.print("[USB] fs.mount() error: ");
      Serial.println(err);
    } else {
      Serial.print("[USB] msd.connect() failed, attempt ");
      Serial.println(i + 1);
    }
    delay(delayMs);
  }
  Serial.println("[USB] Could not mount drive.");
  return false;
}

// ── File listing ────────────────────────────────────────────
// Walks /usb/ and streams a JSON array directly to the client.
// Avoids DynamicJsonDocument entirely, no heap allocation limit,
// no silent truncation if the pool is exhausted.
// CAUTION: While these types are included, the audio playback library
// only supports .wav files currently.
// TODO: Potentially add support for other file formats?
void sendFileListJson(WiFiClient& client) {
  const char* AUDIO_EXTS[] = { ".wav", ".mp3", ".ogg", ".flac", ".aac" };
  const uint8_t EXT_COUNT  = 5;

  // Send headers first so we can stream the body incrementally
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Connection: close");
  client.println();
  client.print("[");

  DIR* dir = opendir("/usb");
  if (!dir) {
    client.print("]");
    return;
  }

  bool first = true;
  struct dirent* entry;
  // Iterate through all present files; if proper format, stream into JSON array
  while ((entry = readdir(dir)) != nullptr) {
    // Skip directories and hidden files
    if (entry->d_type == DT_DIR) continue;
    String name(entry->d_name);
    if (name.startsWith(".")) continue;

    // Check extension (case-insensitive)
    String lower = name;
    lower.toLowerCase();
    bool isAudio = false;
    for (uint8_t e = 0; e < EXT_COUNT; e++) {
      if (lower.endsWith(AUDIO_EXTS[e])) { isAudio = true; break; }
    }
    if (!isAudio) continue;

    // Grab file size via stat
    String path = "/usb/" + name;
    struct stat st;
    long bytes = 0;
    if (stat(path.c_str(), &st) == 0) bytes = st.st_size;

    // Escape any quotes in the filename (shouldn't normally occur, but be safe)
    String escaped = name;
    escaped.replace("\\", "\\\\");
    escaped.replace("\"", "\\\"");

    if (!first) client.print(",");
    client.print("{\"name\":\"");
    client.print(escaped);
    client.print("\",\"bytes\":");
    client.print(bytes);
    client.print("}");
    first = false;
  }
  closedir(dir);

  client.print("]");
}

// HTTP helpers ─────────────────────────────────────────────
void sendJson(WiFiClient& client, int code, const String& body) {
  client.print("HTTP/1.1 "); client.print(code); client.println(" OK");
  client.println("Content-Type: application/json");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Connection: close");
  client.println();
  client.print(body);
}

void sendText(WiFiClient& client, int code, const String& body) {
  client.print("HTTP/1.1 "); client.print(code); client.println(" OK");
  client.println("Content-Type: text/plain");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Connection: close");
  client.println();
  client.print(body);
}

void sendHtml(WiFiClient& client, const char* page) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.print(page);
}

// TCP stream helpers ───────────────────────────────────────
// IMPORTANT: These must be defined BEFORE parseRequest because they
// are marked static - the Arduino IDE does not auto-generate prototypes
// for static functions, so they must appear before any caller.

#define WRITE_BUF 4096

// Read bytes from client into buf, up to `want` bytes.
// Spins without delay() while data is flowing; backs off with a
// short yield only when the TCP buffer is empty.
// Returns number of bytes read (may be less than want on disconnect).
static int tcpRead(WiFiClient& client, uint8_t* buf, int want,
                   unsigned long idleTimeoutMs = 30000) {
  int got = 0;
  unsigned long lastByte = millis();
  while (got < want) {
    if (client.available()) {
      // Drain as many bytes as available in one shot
      int chunk = client.read(buf + got, want - got);
      if (chunk > 0) {
        got += chunk;
        lastByte = millis();
      }
    } else {
      if (!client.connected())                break;
      if (millis() - lastByte > idleTimeoutMs) break;
      // TCP buffer dry, yield one tick then retry
      delay(1);
    }
  }
  return got;
}

// Read one text line (up to \n) from client.
// Returns false on timeout or disconnect.
static bool readLine(WiFiClient& client, String& out,
                     unsigned long timeoutMs = 5000) {
  out = "";
  unsigned long t = millis();
  while (true) {
    if (client.available()) {
      char c = (char)client.read();
      t = millis();
      if (c == '\n') return true;
      if (c != '\r') out += c;
    } else {
      if (!client.connected())      return false;
      if (millis() - t > timeoutMs) return false;
    }
  }
}

// Request parsing ─────────────────────────────────────────
// Named HttpRequest (not Request) to avoid collision with mbed RTOS internals.
// moved (struct and prototype are declared at top of file)

HttpRequest parseRequest(WiFiClient& client) {
  HttpRequest req;
  bool headersDone = false;

  // Header parsing for HTTP request, body read below
  while (client.connected()) {
    if (!client.available()) continue;

    // Read one line of the header at a time
    String line = client.readStringUntil('\n');
    line.trim();

    if (!headersDone) {
      if (line.length() == 0) {
        // Blank line = end of headers, body handled after loop
        headersDone = true;
        break;
      }

      // First line: "GET /path?query HTTP/1.1"
      // If method is empty, it's our first iteration
      // No version extraction because we can expect that to be constant
      if (req.method.length() == 0) {
        int s1 = line.indexOf(' ');
        int s2 = line.indexOf(' ', s1 + 1);
        // Make sure the line is formatted correctly
        if (s1 > 0 && s2 > s1) {
          // Method assignment
          req.method = line.substring(0, s1);
          String fullPath = line.substring(s1 + 1, s2);
          int q = fullPath.indexOf('?');
          // If there is a query (text after PATH) then add to query member
          if (q >= 0) {
            req.path  = fullPath.substring(0, q);
            req.query = fullPath.substring(q + 1);
          } else {
            req.path = fullPath;
          }
        }
      }

      // Content-Length header
      if (line.startsWith("Content-Length:")) {
        req.contentLength = line.substring(15).toInt();
      }

      // Content-Type header (needed for multipart boundary)
      if (line.startsWith("Content-Type:")) {
        req.contentType = line.substring(13);
        req.contentType.trim();
      }
    }
  }

  // For multipart uploads we do NOT buffer the body here -
  // handleUpload reads the stream directly after headers.
  // For all other POST bodies (JSON, etc.) read into a char buffer
  // then assign to String - much faster than byte-by-byte append.
  // Capped at 32 KB to protect heap, config JSON is < 8 KB in practice.
  if (req.method == "POST" && req.contentLength > 0
      && req.contentType.indexOf("multipart") < 0) {
    long toRead = min(req.contentLength, 32768L);
    char* buf   = (char*)malloc(toRead + 1);
    if (buf) {
      int got  = tcpRead(client, (uint8_t*)buf, (int)toRead, 8000);
      buf[got] = '\0';
      req.body = String(buf);
      free(buf);
    }
  }

  return req;
}

// Route handlers ───────────────────────────────────────────

// GET /ping "OK"
void handlePing(WiFiClient& client) {
  sendText(client, 200, "OK");
}

// "Simple" JSONization. Add parameters to doc, serialize and send.
// GET /settings { serial, firmware, time, usbMounted }
void handleSettings(WiFiClient& client) {
  StaticJsonDocument<256> doc;
  doc["serial"]     = SERIAL_NUM;
  doc["firmware"]   = FIRMWARE;
  doc["time"]       = getLocaltime();
  doc["usbMounted"] = usbMounted;

  String json;
  serializeJson(doc, json);
  sendJson(client, 200, json);
}

// GET /files, streams JSON array of audio files on USB
void handleGetFiles(WiFiClient& client) {
  if (!usbMounted) {
    sendJson(client, 503, "{\"error\":\"USB drive not mounted\"}");
    return;
  }
  sendFileListJson(client);
}

// POST /synctime body: { year, month, day, hour, minute, second }
void handleSyncTime(WiFiClient& client, const String& body) {
  StaticJsonDocument<200> doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    sendText(client, 400, "Invalid JSON");
    return;
  }

  tm t = {};
  t.tm_year  = (int)doc["year"]   - 1900;
  t.tm_mon   = (int)doc["month"]  - 1;
  t.tm_mday  = (int)doc["day"];
  t.tm_hour  = (int)doc["hour"];
  t.tm_min   = (int)doc["minute"];
  t.tm_sec   = (int)doc["second"];
  t.tm_isdst = 0;
  set_time(mktime(&t));

  Serial.print("[RTC] Time set to: ");
  Serial.println(getLocaltime());

  sendText(client, 200, "OK");
}

// Config file - read/write /usb/config.json ────────────────
// Example format:
// {
//   "schedule": [
//     ["hawk.wav","","hawk.wav", ... ],   // 24 entries, day 0 (Mon)
//     [...],                              // day 1 (Tue) … day 6 (Sun)
//   ],
//   "doNotPlay": [
//     { "date": "2026-04-15", "note": "Graduation" },
//     ...
//   ]
// }
//
// Empty string = silent slot. Filenames only -- no browser-side IDs.
// Writes are atomic: write to .tmp then rename, so a power failure
// during write never corrupts the previous good config.

// GET /config
// Returns the raw config.json contents, or an empty-schedule default
// if the file doesn't exist yet (first boot).
void handleGetConfig(WiFiClient& client) {
  if (!usbMounted) {
    sendJson(client, 503, "{\"error\":\"USB not mounted\"}");
    return;
  }

  FILE* fp = fopen(CONFIG_PATH, "r");
  if (!fp) {
    // First boot, return a valid empty config so the browser
    // initialises cleanly without a separate error path.
    Serial.println("[CONFIG] No config.json found, returning empty default");
    sendJson(client, 200,
      "{\"schedule\":[[],[],[],[],[],[],[]],"
      "\"doNotPlay\":[]}");
    return;
  }

  // Stream file directly to client in 512-byte chunks.
  // Config files are small (< 20 KB typically) so this is fast.
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Connection: close");
  client.println();

  uint8_t chunk[512];
  size_t n;
  while ((n = fread(chunk, 1, sizeof(chunk), fp)) > 0) {
    client.write(chunk, n);
  }
  fclose(fp);
}

// POST /config
// Receives the full config JSON, validates it minimally, then writes
// atomically via a .tmp file + rename.
void handlePostConfig(WiFiClient& client, const String& body) {
  if (!usbMounted) {
    sendJson(client, 503, "{\"error\":\"USB not mounted\"}");
    return;
  }

  if (body.length() == 0) {
    sendText(client, 400, "Empty body");
    return;
  }

  // Quick structural validation, just check the two required keys exist.
  // Full deserialisation would need ~8 KB of stack/heap for a full schedule;
  // we trust the browser to send well-formed JSON and rely on rename
  // atomicity for safety instead.
  if (body.indexOf("\"schedule\"") < 0 || body.indexOf("\"doNotPlay\"") < 0) {
    sendText(client, 400, "Missing required keys");
    return;
  }

  // Write to .tmp first
  FILE* fp = fopen(CONFIG_TMP_PATH, "w");
  if (!fp) {
    sendText(client, 500, "Cannot open tmp file");
    return;
  }

  size_t written = fwrite(body.c_str(), 1, body.length(), fp);
  fclose(fp);

  if ((int)written != (int)body.length()) {
    remove(CONFIG_TMP_PATH);
    sendText(client, 500, "Write error");
    return;
  }

  // Atomic rename.
  // FAT doesn't guarantee rename atomicity across clusters, but it's
  // far safer than writing directly to config.json, a partial write
  // to .tmp never touches the good file.
  remove(CONFIG_PATH);                     // FAT rename fails if dest exists
  if (rename(CONFIG_TMP_PATH, CONFIG_PATH) != 0) {
    sendText(client, 500, "Rename failed");
    return;
  }

  Serial.print("[CONFIG] Saved - ");
  Serial.print(written);
  Serial.println(" bytes");

  // Keep the in-memory schedule matrix in sync so the new schedule
  // takes effect immediately without requiring a reboot
  loadScheduleFromConfig();

  sendJson(client, 200, "{\"ok\":true}");
}

// Multipart upload - exact-byte streaming to USB ───────────
//
// Key insight: we know contentLength (full multipart body size) and
// we can measure the framing overhead precisely by counting the bytes
// consumed reading the part-headers. That gives us an exact file byte
// count to read - no boundary scanning of the file body needed.
//
// Layout of the multipart body after HTTP headers:
//
//   --boundary\r\n                        opening boundary line
//   Content-Disposition: ...\r\n         
//   Content-Type: audio/wav\r\n           part headers  (variable)
//   \r\n                                  blank line
//   [FILE BYTES]                          exactly fileBytes bytes
//   \r\n--boundary--\r\n                  closing delimiter
//
// framing = openingLine + partHeaders + blankLine + closingDelimiter
// fileBytes = contentLength - framing  (calculated once headers parsed)
//
// We then read exactly fileBytes from the TCP stream in WRITE_BUF
// chunks, writing each chunk straight to disk.
//
// Write buffer: 4 KB gives ~8× fewer FAT transactions vs 512 B.
// No delay() in the read loop - spin tightly and only yield when
// genuinely dry, using a per-byte activity watchdog.

void handleUpload(WiFiClient& client, const String& contentType, long contentLength) {

  // 1. Extract boundary ──────────────────────────────────
  int bPos = contentType.indexOf("boundary=");
  if (bPos < 0) { sendText(client, 400, "Missing boundary"); return; }
  String boundary = "--" + contentType.substring(bPos + 9);
  boundary.trim();

  Serial.print("[UPLOAD] boundary="); Serial.println(boundary);
  Serial.print("[UPLOAD] contentLength="); Serial.println(contentLength);

  if (!usbMounted) { sendText(client, 503, "USB not mounted"); return; }

  // 2. Read part-headers, measure framing overhead ───────
  //
  // We count every byte consumed here so we can subtract from
  // contentLength to get the exact file byte count.

  int framingBytes = 0;

  // Opening boundary line: "--boundary\r\n"
  // (parseRequest left the stream right after the HTTP blank line,
  //  so the next thing in the stream IS the opening boundary line)
  {
    String line;
    if (!readLine(client, line)) { sendText(client, 408, "Timeout on boundary line"); return; }
    framingBytes += line.length() + 2; // +2 for \r\n
  }

  // Part headers (Content-Disposition, Content-Type, blank line)
  String filename = "";
  while (true) {
    String line;
    if (!readLine(client, line)) { sendText(client, 408, "Timeout reading part headers"); return; }
    framingBytes += line.length() + 2; // +2 for \r\n

    if (line.length() == 0) break; // blank line = end of part headers

    if (line.startsWith("Content-Disposition")) {
      int fnIdx = line.indexOf("filename=\"");
      if (fnIdx >= 0) {
        int start = fnIdx + 10;
        int end   = line.indexOf('"', start);
        if (end > start) filename = line.substring(start, end);
      }
    }
  }

  if (filename.length() == 0) { sendText(client, 400, "No filename in part headers"); return; }

  // Closing delimiter: "\r\n--boundary--\r\n"
  int closingLen = 2 + boundary.length() + 2 + 2; // \r\n + "--boundary--" + \r\n
  framingBytes += closingLen;

  // Exact file byte count
  long fileBytes = (long)contentLength - framingBytes;

  Serial.print("[UPLOAD] framingBytes="); Serial.println(framingBytes);
  Serial.print("[UPLOAD] fileBytes=");    Serial.println(fileBytes);

  if (fileBytes <= 0) { sendText(client, 400, "Computed file size <= 0"); return; }

  // 3. Clean filename and open file ───────────────────
  String safeName = "";
  for (char c : filename) {
    if (isAlphaNumeric(c) || c == '.' || c == '_' || c == '-') safeName += c;
  }
  if (safeName.length() == 0) safeName = "upload.bin";

  String filepath = "/usb/" + safeName;
  Serial.print("[UPLOAD] Writing to: "); Serial.println(filepath);

  FILE* fp = fopen(filepath.c_str(), "wb");
  if (!fp) { sendText(client, 500, "Cannot open file for writing"); return; }

  // 4. Stream exactly fileBytes to disk ──────────────────
  //
  // Read in WRITE_BUF chunks. tcpRead() handles the TCP idle gap
  // without delay()-per-byte overhead.
  // Idle timeout scales with file size: at least 30 s, plus 10 s
  // per MB - so a 50 MB file gets ~8 minutes, a 1 MB file gets 40 s.

  uint8_t* buf = (uint8_t*)malloc(WRITE_BUF);
  if (!buf) { fclose(fp); sendText(client, 500, "OOM"); return; }

  unsigned long idleTimeout = max(30000UL,
    30000UL + (unsigned long)(fileBytes / (1024 * 1024)) * 10000UL);

  Serial.print("[UPLOAD] idleTimeout="); Serial.print(idleTimeout / 1000); Serial.println("s");

  long remaining = fileBytes;
  long written   = 0;
  bool ioError   = false;

  while (remaining > 0) {
    int want = (int)min((long)WRITE_BUF, remaining);
    int got  = tcpRead(client, buf, want, idleTimeout);

    if (got > 0) {
      size_t w = fwrite(buf, 1, got, fp);
      written   += (long)w;
      remaining -= (long)got;
      if ((int)w != got) {
        Serial.println("[UPLOAD] fwrite error");
        ioError = true; break;
      }
    }

    if (got < want) {
      // Received fewer bytes than requested, disconnected or timed out
      Serial.print("[UPLOAD] Short read: got="); Serial.print(got);
      Serial.print(" want="); Serial.println(want);
      ioError = true; break;
    }
  }

  free(buf);
  fclose(fp);

  Serial.print("[UPLOAD] Bytes written: "); Serial.println(written);

  // 5. Validate and respond ───────────────────────────────
  if (ioError || written != fileBytes) {
    Serial.print("[UPLOAD] Incomplete - removing. written=");
    Serial.print(written); Serial.print(" expected="); Serial.println(fileBytes);
    remove(filepath.c_str());
    sendText(client, 500, "Upload incomplete - partial file removed");
    return;
  }

  // Drain the closing delimiter from the stream (don't care about content)
  // so the TCP connection closes cleanly.
  {
    uint8_t drain[8];
    tcpRead(client, drain, min(closingLen, 8), 3000);
  }

  Serial.print("[UPLOAD] Done: "); Serial.println(safeName);

  StaticJsonDocument<128> doc;
  doc["name"]  = safeName;
  doc["bytes"] = written;
  String json;
  serializeJson(doc, json);
  sendJson(client, 200, json);
}

// Main page ────────────────────────────────────────────────
// The full HTML/CSS/JS page is stored in flash as a raw string.
// Keep it in a separate file (page.h) and #include it here.
// For now it is declared extern – see page.h.
extern const char PAGE[];

void handleRoot(WiFiClient& client) {
  sendHtml(client, PAGE);
}

// Router ───────────────────────────────────────────────────
void routeRequest(WiFiClient& client) {
  // Passes to parseRequest() to fill the request struct, members compared here
  HttpRequest req = parseRequest(client);

  Serial.print("[HTTP] ");
  Serial.print(req.method);
  Serial.print(" ");
  Serial.println(req.path);

  if      (req.method == "GET"  && req.path == "/ping")      handlePing(client);                                        // Connection status checking
  else if (req.method == "GET"  && req.path == "/settings")  handleSettings(client);                                    // Send settings to website for display
  else if (req.method == "GET"  && req.path == "/files")     handleGetFiles(client);                                    // Streams JSON array of audio files to website
  else if (req.method == "GET"  && req.path == "/config")    handleGetConfig(client);                                   // Send persisted schedule + do-not-play to website
  else if (req.method == "POST" && req.path == "/config")    handlePostConfig(client, req.body);                        // Receive and persist schedule + do-not-play from website
  else if (req.method == "POST" && req.path == "/synctime")  handleSyncTime(client, req.body);                          // POST as client will post back to server the peripheral time
  else if (req.method == "POST" && req.path == "/upload")    handleUpload(client, req.contentType, req.contentLength);  // POST as audio file data is coming to the arduino
  else if (req.method == "GET"  && req.path == "/")          handleRoot(client);                                        // Sends the webpage when needed
  else sendText(client, 404, "Not found");

  delay(1);
  client.stop();
}

// Audio playback subsystem ─────────────────────────────────
//
// Design goals:
//   1. Non-blocking - playbackTick() feeds one DAC buffer per call
//      and returns immediately, so loop() stays responsive for WiFi.
//   2. Single-file, play-once - startPlayback() opens a WAV file,
//      stopPlayback() closes it cleanly.  The DAC is only initialised
//      when a file is scheduled, then stopped when the file ends.
//   3. Stereo WAV files are averaged to mono (same as the reference
//      example) since a single DAC output (A12) is used.
//
// Hour-trigger logic:
//   checkSchedule() is called from loop().  It reads the current RTC
//   hour and compares against lastPlayedHour.  When the hour changes
//   it looks up the schedule for that day/hour and, if a file is
//   assigned and present on the USB drive, starts playback.
//   Future models will replace this with an API alarm.

#define DAC_PIN       A12
#define DAC_N_SAMPLES 512   // DMA buffer size - must match wavreader.begin()

AdvancedDAC   dac0(DAC_PIN);
WavReader     wavreader;

// Playback state
static bool   playbackActive = false;
static String playbackFile   = "";   // currently open file path

// Hour-trigger state, initialised to 0xFF so the first real hour always fires
static uint8_t lastPlayedHour = 0xFF;

// Schedule storage (loaded from config.json) ───────────────
// Mirrors the browser-side schedule[][]: filename per day/hour.
// Populated by loadScheduleFromConfig() at boot and on each
// POST /config.  Indexed [day 0-6][hour 0-23].
// Day 0 = Monday, matching the web UI convention.
static String scheduleMatrix[7][24];

// Load schedule from config.json into scheduleMatrix ───────
// Called once at boot (after USB mounts) and after every
// successful POST /config so the in-memory matrix stays in sync.
void loadScheduleFromConfig() {
  // Clear matrix first so stale entries don't persist if the file
  // is missing or malformed
  for (int d = 0; d < 7; d++)
    for (int h = 0; h < 24; h++)
      scheduleMatrix[d][h] = "";

  FILE* fp = fopen(CONFIG_PATH, "r");
  if (!fp) {
    Serial.println("[SCHED] No config.json - schedule matrix empty");
    return;
  }

  // Read entire file into a String for ArduinoJson parsing.
  // Config is < 8 KB so this is safe.
  fseek(fp, 0, SEEK_END);
  long sz = ftell(fp);
  rewind(fp);

  char* buf = (char*)malloc(sz + 1);
  if (!buf) { fclose(fp); Serial.println("[SCHED] OOM reading config"); return; }
  fread(buf, 1, sz, fp);
  buf[sz] = '\0';
  fclose(fp);

  DynamicJsonDocument doc(8192);
  DeserializationError err = deserializeJson(doc, buf);
  free(buf);

  if (err) {
    Serial.print("[SCHED] JSON parse error: ");
    Serial.println(err.c_str());
    return;
  }

  JsonArray sched = doc["schedule"].as<JsonArray>();
  for (int d = 0; d < 7 && d < (int)sched.size(); d++) {
    JsonArray day = sched[d].as<JsonArray>();
    for (int h = 0; h < 24 && h < (int)day.size(); h++) {
      scheduleMatrix[d][h] = day[h].as<String>();
    }
  }

  Serial.println("[SCHED] Schedule matrix loaded from config.json");
}

// Start playback of a file ──────────────────────────────────
// Returns true if the DAC and WavReader both initialised successfully.
bool startPlayback(const String& filename) {
  if (playbackActive) stopPlayback();

  String path = "/usb/" + filename;
  Serial.print("[AUDIO] Opening: "); Serial.println(path);

  // WavReader.begin(path, n_samples, channel_count_hint, loop)
  // channel_count_hint=1 tells it we want mono output, it still
  // reads stereo files correctly and we average in playbackTick().
  if (!wavreader.begin(path.c_str(), DAC_N_SAMPLES, 1, false)) {
    Serial.println("[AUDIO] WavReader open failed");
    return false;
  }

  Serial.print("[AUDIO] Channels: ");    Serial.println(wavreader.channels());
  Serial.print("[AUDIO] Sample rate: "); Serial.println(wavreader.sample_rate());
  Serial.print("[AUDIO] Samples: ");     Serial.println(wavreader.sample_count());

  // Initialise DAC at the file's native sample rate.
  // Queue depth of 32 gives ~32 × 512 / sample_rate seconds of buffer,
  // enough headroom for the web server to briefly take the loop.
  if (!dac0.begin(AN_RESOLUTION_12, wavreader.sample_rate(),
                  DAC_N_SAMPLES, 32)) {
    Serial.println("[AUDIO] DAC init failed");
    wavreader.stop();
    return false;
  }

  playbackActive = true;
  playbackFile   = filename;
  Serial.println("[AUDIO] Playback started");
  return true;
}

// Stop playback and release DAC ────────────────────────────
void stopPlayback() {
  if (!playbackActive) return;
  dac0.stop();
  wavreader.stop();
  playbackActive = false;
  playbackFile   = "";
  Serial.println("[AUDIO] Playback stopped");
}

// Feed one DAC buffer, call every loop() iteration ────────
// Returns immediately if playback is not active or the DAC queue
// is full. This keeps the web server responsive.
void playbackTick() {
  if (!playbackActive) return;

  // File exhausted, stop cleanly
  if (!wavreader.available()) {
    Serial.println("[AUDIO] File complete");
    stopPlayback();
    return;
  }

  // Only feed if the DAC has a free buffer slot (non-blocking check)
  if (!dac0.available()) return;

  SampleBuffer dacbuf = dac0.dequeue();
  SampleBuffer pcmbuf = wavreader.read();

  const int channels = wavreader.channels();
  for (size_t i = 0; i < DAC_N_SAMPLES; i++) {
    if (channels == 1) {
      // Mono: shift signed 16-bit to unsigned 12-bit
      dacbuf[i] = ((unsigned int)((int16_t)pcmbuf[i] + 32768)) >> 4;
    } else {
      // Stereo: average L+R channels then convert
      int32_t avg = ((int16_t)pcmbuf[i * 2] + (int16_t)pcmbuf[i * 2 + 1]) / 2;
      dacbuf[i]   = ((unsigned int)(avg + 32768)) >> 4;
    }
  }

  dac0.write(dacbuf);
  pcmbuf.release();
}

// Hour schedule check, call every loop() iteration ────────
// Reads current RTC time. If the hour has changed since the last
// check AND that day/hour has a file assigned, starts playback.
// Also checks do-not-play dates against today's date.
//
// CAUTION: This is a software poll, adequate for hourly granularity.
// Future replacement with an API call to allow for software sleep.
void checkSchedule() {
  if (!usbMounted) return;

  tm t;
  _rtc_localtime(time(NULL), &t, RTC_4_YEAR_LEAP_YEAR_SUPPORT);

  uint8_t currentHour = (uint8_t)t.tm_hour;
  if (currentHour == lastPlayedHour) return; // same hour, nothing to do
  lastPlayedHour = currentHour;

  // tm_wday: 0=Sunday … 6=Saturday.
  // scheduleMatrix uses 0=Monday … 6=Sunday to match the web UI.
  // Convert: (tm_wday + 6) % 7  maps Sun(0)->6, Mon(1)->0, … Sat(6)->5
  int dayIndex = (t.tm_wday + 6) % 7;

  // Check do-not-play list in config.json
  // Build today's date string (YYYY-MM-DD) for comparison
  char todayStr[11];
  snprintf(todayStr, sizeof(todayStr), "%04d-%02d-%02d",
           t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

  // Quick scan of the raw JSON for the date, avoids a full re-parse
  // CAUTION: This is a simple string search. It will match partial
  // dates if a note happens to contain the date string. For production
  // a proper JSON parse of the doNotPlay array would be more robust.
  // TODO: Parse doNotPlay properly once tested.
  FILE* fp = fopen(CONFIG_PATH, "r");
  if (fp) {
    char jsonBuf[256];
    bool dnpMatch = false;
    while (fgets(jsonBuf, sizeof(jsonBuf), fp)) {
      if (strstr(jsonBuf, todayStr)) { dnpMatch = true; break; }
    }
    fclose(fp);
    if (dnpMatch) {
      Serial.print("[SCHED] Do-not-play date matched: ");
      Serial.println(todayStr);
      return;
    }
  }

  String filename = scheduleMatrix[dayIndex][currentHour];
  if (filename.length() == 0) {
    Serial.print("[SCHED] Hour ");
    Serial.print(currentHour);
    Serial.println(" is silent");
    return;
  }

  // Verify file exists before attempting playback
  String filepath = "/usb/" + filename;
  FILE* testFp = fopen(filepath.c_str(), "r");
  if (!testFp) {
    Serial.print("[SCHED] Scheduled file not found: ");
    Serial.println(filepath);
    return;
  }
  fclose(testFp);

  Serial.print("[SCHED] Hour "); Serial.print(currentHour);
  Serial.print(" -> "); Serial.println(filename);
  startPlayback(filename);
}

// setup / loop ─────────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  while (!Serial) { /* wait for USB-CDC */ }

  Serial.println("=== Bird Deterrent starting ===");

  // Begin RTC
  rtcSetDefault();

  // Status LEDs
  pinMode(LED_RED,   OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE,  OUTPUT);

  // WiFi AP
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("FATAL: No WiFi module");
    while (true) {}
  }

  Serial.print("Creating AP: ");
  Serial.println(ssid);
  // Startup on Access Point
  int wifiStatus = WiFi.beginAP(ssid, pass);
  if (wifiStatus != WL_AP_LISTENING) {
    Serial.println("FATAL: AP failed");
    while (true) {}
  }

  delay(2000); // give AP a moment

  // USB drive
  usbMounted = mountUSB();

  // Load schedule matrix into RAM so checkSchedule() can use it
  // without re-parsing JSON on every loop iteration
  if (usbMounted) loadScheduleFromConfig();

  // Start HTTP server
  server.begin();
  Serial.print("Server at http://");
  Serial.println(WiFi.localIP());
}

// Our loop monitors the client to ensure server connection;
// if the client sends information it's passed to routeRequest.
// playbackTick() feeds the DAC non-blockingly on every iteration.
// checkSchedule() fires at most once per hour (cheap RTC comparison).
void loop() {
  // Feed DAC if playback is active, must run every iteration
  playbackTick();

  // Check if a new hour has arrived and trigger scheduled playback
  checkSchedule();

  // Handle incoming HTTP request if a client is connected
  WiFiClient client = server.available();
  if (client) routeRequest(client);
}
