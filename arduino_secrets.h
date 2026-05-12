#include <Arduino.h>

#define SECRET_SSID "placeholder_name"        // For a different network name, change the text in quotes
#define SECRET_PASS "placeholder_password"    // For a different network password, change the text in quotes

const char PAGE[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <title>Bird Deterrent – Control Panel</title>
  <style>
    :root{
      --bg:#FBF7EF;
      --fg:#1F2937;
      --muted:#EEF2F1;
      --mutedFg:#5B6B63;
      --card:#FFFFFF;
      --border:#D8E0DB;
      --input:#FFFFFF;
      --accent:#2F6F55;
      --accentFg:#FFFFFF;
      --secondary:#3A6EA5;
      --tertiary:#B7794A;
      --quaternary:#79A56B;
      --ring:#2F6F55;
      --bad:#B42318;
      --bw:2px;
      --r-sm:8px;
      --r-md:16px;
      --r-lg:24px;
      --r-full:9999px;
      --shadow-pop:4px 4px 0 0 var(--fg);
      --shadow-pop-hover:6px 6px 0 0 var(--fg);
      --shadow-pop-active:2px 2px 0 0 var(--fg);
      --shadow-card:8px 8px 0 0 var(--border);
      --shadow-card-feature:8px 8px 0 0 rgba(121,165,107,.55);
      --max:980px;
      --pad:14px;
      --heading: "Outfit", system-ui, -apple-system, Segoe UI, Roboto, Helvetica, Arial, sans-serif;
      --body: "Plus Jakarta Sans", system-ui, -apple-system, Segoe UI, Roboto, Helvetica, Arial, sans-serif;
      --mono: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono", "Courier New", monospace;
      --ease-pop: cubic-bezier(0.34,1.56,0.64,1);
    }
    *{box-sizing:border-box}
    html,body{margin:0}
    body{
      font-family:var(--body);
      color:var(--fg);
      background:
        radial-gradient(1200px 800px at 12% 0%, rgba(58,110,165,.12), transparent 60%),
        radial-gradient(980px 700px at 88% 20%, rgba(47,111,85,.12), transparent 62%),
        radial-gradient(900px 680px at 60% 110%, rgba(183,121,74,.10), transparent 58%),
        linear-gradient(180deg, var(--bg) 0%, #F6F1E6 100%);
      -webkit-font-smoothing:antialiased;
      -moz-osx-font-smoothing:grayscale;
    }
    body:before{
      content:"";
      position:fixed;
      inset:0;
      pointer-events:none;
      opacity:.45;
      background-image: radial-gradient(rgba(31,41,55,.16) 1px, transparent 1px);
      background-size: 18px 18px;
      background-position: 0 2px;
      mix-blend-mode:multiply;
    }
    a{color:inherit}
    .wrap{max-width:var(--max); margin:0 auto; padding:16px 12px 28px; position:relative;}
    header{display:flex; align-items:flex-start; justify-content:space-between; gap:12px; padding:10px 4px 14px; position:relative;}
    .brand{display:flex; flex-direction:column; gap:8px;}
    .title{display:flex; align-items:center; gap:12px;}
    .logo{
      width:44px; height:44px;
      border-radius: 18px 18px 18px 4px;
      background: linear-gradient(135deg, rgba(47,111,85,.95), rgba(58,110,165,.88));
      border:var(--bw) solid var(--fg);
      box-shadow:var(--shadow-pop);
      position:relative;
      overflow:hidden;
      flex:0 0 auto;
    }
    .logo:after{
      content:"";
      position:absolute;
      left:10px; top:10px;
      width:10px; height:10px;
      border-radius:var(--r-full);
      background:rgba(255,255,255,.90);
      box-shadow: 14px 2px 0 rgba(255,255,255,.90), 6px 16px 0 rgba(255,255,255,.90), 20px 18px 0 rgba(255,255,255,.90);
      opacity:.85;
    }
    h1{font-family:var(--heading); font-size:16px; line-height:1.15; margin:0; letter-spacing:.2px; font-weight:800;}
    .sub{font-size:12px; color:var(--mutedFg); line-height:1.25; max-width:54ch;}
    .chips{display:flex; gap:8px; flex-wrap:wrap;}
    .chip{
      display:inline-flex; align-items:center; gap:8px;
      padding:8px 10px;
      border-radius:var(--r-full);
      border:var(--bw) solid var(--fg);
      background:var(--card);
      box-shadow:var(--shadow-pop);
      font-size:12px;
      user-select:none;
    }
    .chip .dot{width:10px;height:10px;border-radius:var(--r-full);border:var(--bw) solid var(--fg);transition: background-color 0.3s ease;}
    .dot.connected    { background-color: rgba(47,111,85,.85); }
    .dot.disconnected { background-color: rgba(180,50,50,.85); }
    .dot.warning      { background-color: rgba(183,121,74,.85); }
    .mono{font-family:var(--mono)}
    .devicePill{
      display:inline-flex; align-items:center; gap:10px;
      padding:10px 12px;
      border-radius:var(--r-full);
      border:var(--bw) solid var(--fg);
      background:var(--card);
      box-shadow:var(--shadow-pop);
      font-size:12px;
      user-select:none;
      white-space:nowrap;
      cursor:pointer;
      transition: transform .22s var(--ease-pop), box-shadow .22s var(--ease-pop);
    }
    .devicePill:hover{transform: translate(-2px,-2px); box-shadow:var(--shadow-pop-hover)}
    .devicePill:active{transform: translate(2px,2px); box-shadow:var(--shadow-pop-active)}
    .devicePill .badge{
      display:inline-flex; align-items:center; gap:6px;
      padding:6px 8px;
      border-radius:var(--r-full);
      border:var(--bw) solid var(--fg);
      background:rgba(183,121,74,.28);
      font-weight:800;
    }
    main{display:grid; grid-template-columns:1fr; gap:14px;}
    .card{background:var(--card); border:var(--bw) solid var(--fg); border-radius:var(--r-lg); box-shadow:var(--shadow-card); overflow:hidden; position:relative;}
    .card.feature{box-shadow:var(--shadow-card-feature)}
    .card .head{
      display:flex; align-items:flex-end; justify-content:space-between; gap:10px;
      padding:14px 14px 10px;
      border-bottom:var(--bw) solid var(--fg);
      background: linear-gradient(0deg, rgba(238,242,241,.75), rgba(238,242,241,.75));
    }
    .card .head h2{margin:0; font-family:var(--heading); font-size:13px; letter-spacing:.2px; font-weight:800;}
    .card .head .hint{color:var(--mutedFg); font-size:12px;}
    .card .body{padding:14px;}
    .row{display:flex; gap:10px; align-items:center; flex-wrap:wrap;}
    .stack{display:flex; flex-direction:column; gap:8px;}
    label{font-size:11px; font-weight:800; letter-spacing:.08em; text-transform:uppercase; color:var(--mutedFg); display:block; margin:0 0 6px;}
    input[type="text"], input[type="number"], input[type="date"], input[type="time"], select{
      width:100%; padding:12px 12px; border-radius:var(--r-md); border:var(--bw) solid rgba(31,41,55,.25);
      background:var(--input); color:var(--fg); outline:none;
      transition: box-shadow .22s var(--ease-pop), transform .22s var(--ease-pop), border-color .18s ease;
    }
    input[type="text"]::placeholder{color:rgba(91,107,99,.70)}
    input:focus, select:focus{border-color:var(--ring); box-shadow: 4px 4px 0 0 var(--ring);}
    input[type="file"]{width:100%; padding:8px 0;}
    .btn{
      appearance:none; border:var(--bw) solid var(--fg); background:var(--card); color:var(--fg);
      padding:12px 14px; border-radius:var(--r-full); font-size:12px; font-weight:800; letter-spacing:.02em;
      cursor:pointer; user-select:none; box-shadow:none;
      transition: transform .22s var(--ease-pop), box-shadow .22s var(--ease-pop), background .18s ease;
      min-height:48px;
    }
    .btn:hover{transform: translate(-2px,-2px)}
    .btn:active{transform: translate(2px,2px)}
    .btn.primary{background:var(--accent); color:var(--accentFg); box-shadow:var(--shadow-pop)}
    .btn.primary:hover{box-shadow:var(--shadow-pop-hover)}
    .btn.primary:active{box-shadow:var(--shadow-pop-active)}
    .btn.secondary{background:transparent; box-shadow:none;}
    .btn.secondary:hover{background:rgba(183,121,74,.22)}
    .btn.bad{background:rgba(180,35,24,.10); border-color:var(--fg); box-shadow:var(--shadow-pop)}
    .btn.ghost{background:transparent; border-color:rgba(31,41,55,.35); box-shadow:none;}
    .btn:disabled{opacity:.45; pointer-events:none;}
    .tiny{font-size:11px; color:var(--mutedFg); line-height:1.35;}
    .hr{height:0; border-top:var(--bw) dashed rgba(31,41,55,.35); margin:12px 0;}
    .table{width:100%; border-collapse:separate; border-spacing:0; border-radius:var(--r-lg); overflow:hidden; border:var(--bw) solid var(--fg); background:var(--card); box-shadow: 6px 6px 0 0 rgba(216,224,219,.95);}
    .table th, .table td{padding:12px 12px; border-bottom:var(--bw) solid rgba(31,41,55,.12); font-size:12px; text-align:left; vertical-align:middle;}
    .table th{background:rgba(238,242,241,.85); color:var(--mutedFg); font-weight:800; letter-spacing:.06em; text-transform:uppercase; font-size:11px;}
    .table tr:last-child td{border-bottom:none}
    .seg{display:flex; gap:8px; flex-wrap:wrap; padding:10px; border-radius:var(--r-lg); border:var(--bw) solid var(--fg); background:rgba(238,242,241,.82); box-shadow: 6px 6px 0 0 rgba(216,224,219,.95);}
    .seg button{flex:1; min-width:74px; padding:12px 12px; border-radius:var(--r-full); border:var(--bw) solid var(--fg); background:var(--card); color:var(--fg); font-size:12px; font-weight:800; cursor:pointer; user-select:none; transition: transform .22s var(--ease-pop), box-shadow .22s var(--ease-pop), background .18s ease; min-height:44px;}
    .seg button[aria-selected="true"]{background:rgba(47,111,85,.16); box-shadow:var(--shadow-pop)}
    .scheduleWrap{display:grid; grid-template-columns:1fr; gap:12px;}
    .dayMeta{display:flex; align-items:center; justify-content:space-between; gap:10px; padding:12px; border-radius:var(--r-lg); border:var(--bw) solid var(--fg); background:var(--card); box-shadow:var(--shadow-pop);}
    .dayMeta .left{display:flex; flex-direction:column; gap:2px;}
    .dayMeta .left .d{font-family:var(--heading); font-weight:800; font-size:13px;}
    .dayMeta .left .s{color:var(--mutedFg); font-size:12px;}
    .hours{border-radius:var(--r-lg); border:var(--bw) solid var(--fg); overflow:hidden; background:var(--card); box-shadow:var(--shadow-card);}
    .hrow{display:grid; grid-template-columns:66px 1fr; gap:10px; padding:10px 12px; border-bottom:var(--bw) solid rgba(31,41,55,.10); align-items:center;}
    .hrow:last-child{border-bottom:none}
    .hrow .t{font-family:var(--mono); font-size:12px; color:var(--mutedFg);}
    .assign{display:flex; align-items:center; justify-content:space-between; gap:10px; padding:10px 12px; border-radius: 18px 18px 18px 6px; border:var(--bw) solid var(--fg); background: rgba(238,242,241,.62); box-shadow: 4px 4px 0 0 rgba(31,41,55,.18); min-height:48px; cursor:pointer; transition: transform .22s var(--ease-pop), box-shadow .22s var(--ease-pop);}
    .assign:hover{transform: translate(-2px,-2px); box-shadow: 6px 6px 0 0 rgba(31,41,55,.18)}
    .assign:active{transform: translate(2px,2px); box-shadow: 2px 2px 0 0 rgba(31,41,55,.18)}
    .assign .left{display:flex; flex-direction:column; gap:2px; min-width:0;}
    .assign .name{font-weight:800; font-size:12px; white-space:nowrap; overflow:hidden; text-overflow:ellipsis;}
    .assign .meta{font-size:11px; color:var(--mutedFg);}
    .tag{display:inline-flex; align-items:center; gap:6px; padding:6px 10px; border-radius:var(--r-full); border:var(--bw) solid var(--fg); background:var(--card); font-weight:800; font-size:11px; white-space:nowrap;}
    .tag.silent{background:rgba(91,107,99,.12)}
    /* Missing file tag — red tint to draw attention */
    .tag.missing{background:rgba(180,35,24,.12); color:var(--bad); border-color:var(--bad);}
    /* USB warning banner */
    .banner{
      display:flex; align-items:center; gap:12px;
      padding:12px 14px;
      border-radius:var(--r-lg);
      border:var(--bw) solid var(--fg);
      background:rgba(183,121,74,.14);
      box-shadow:var(--shadow-pop);
      font-size:12px;
      font-weight:700;
    }
    .banner.hidden{display:none}
    /* Modal */
    .backdrop{position:fixed; inset:0; background:rgba(31,41,55,.35); display:none; align-items:flex-end; justify-content:center; padding:14px; z-index:999;}
    .backdrop.show{display:flex}
    .sheet{width:min(var(--max), 100%); max-height:82vh; overflow:auto; border-radius: 26px 26px 26px 10px; border:var(--bw) solid var(--fg); background:var(--card); box-shadow: 10px 10px 0 0 rgba(216,224,219,.95);}
    .sheetHead{position:sticky; top:0; display:flex; align-items:center; justify-content:space-between; gap:10px; padding:12px; border-bottom:var(--bw) solid var(--fg); background:rgba(238,242,241,.90); backdrop-filter: blur(6px); z-index:2;}
    .sheetHead .ttl{font-family:var(--heading); font-size:13px; font-weight:800;}
    .sheetBody{padding:12px}
    .kvs{display:grid; grid-template-columns:1fr; gap:10px;}
    .kv{display:flex; align-items:center; justify-content:space-between; gap:10px; padding:12px; border-radius:var(--r-lg); border:var(--bw) solid var(--fg); background:rgba(238,242,241,.70); box-shadow: 6px 6px 0 0 rgba(216,224,219,.95);}
    .kv .k{color:var(--mutedFg); font-size:12px; font-weight:800; letter-spacing:.06em; text-transform:uppercase;}
    .kv .v{font-size:12px; font-weight:700;}
    .pickList{display:flex; flex-direction:column; gap:10px;}
    .pickItem{display:flex; align-items:center; justify-content:space-between; gap:10px; padding:12px; border-radius:var(--r-lg); border:var(--bw) solid var(--fg); background:var(--card); box-shadow:var(--shadow-pop); cursor:pointer; transition: transform .22s var(--ease-pop), box-shadow .22s var(--ease-pop);}
    .pickItem:hover{transform: translate(-2px,-2px); box-shadow:var(--shadow-pop-hover)}
    .pickItem:active{transform: translate(2px,2px); box-shadow:var(--shadow-pop-active)}
    .pickItem .n{font-weight:800; font-size:12px; max-width:58vw; white-space:nowrap; overflow:hidden; text-overflow:ellipsis;}
    .pickItem .m{font-size:11px; color:var(--mutedFg); font-family:var(--mono)}
    .pickItem .right{display:flex; align-items:center; gap:8px;}
    .swatch{width:18px; height:18px; border-radius:var(--r-full); border:var(--bw) solid var(--fg);}
    .swatch.v{background:rgba(47,111,85,.30)}
    .swatch.p{background:rgba(58,110,165,.28)}
    .swatch.y{background:rgba(183,121,74,.26)}
    .swatch.g{background:rgba(121,165,107,.28)}
    .toast{position:fixed; left:50%; bottom:14px; transform:translateX(-50%); display:none; gap:10px; align-items:center; padding:10px 12px; border-radius:var(--r-full); border:var(--bw) solid var(--fg); background:var(--card); box-shadow:var(--shadow-pop); z-index:1000; font-size:12px; max-width:92vw; font-weight:800;}
    .toast.show{display:flex}
    /* ── Upload UI ───────────────────────────────────────────── */
    .uploadZone{
      display:flex;
      flex-direction:column;
      align-items:center;
      justify-content:center;
      gap:8px;
      padding:20px 14px;
      border-radius:var(--r-lg);
      border:var(--bw) dashed rgba(31,41,55,.35);
      background:rgba(238,242,241,.55);
      cursor:pointer;
      transition: border-color .18s ease, background .18s ease;
      text-align:center;
      min-height:90px;
      position:relative;
    }
    .uploadZone:hover, .uploadZone.drag{
      border-color:var(--accent);
      background:rgba(47,111,85,.07);
    }
    .uploadZone input[type="file"]{
      position:absolute; inset:0; opacity:0; cursor:pointer; width:100%; height:100%;
    }
    .uploadZone .uz-icon{font-size:22px; line-height:1;}
    .uploadZone .uz-label{font-size:12px; font-weight:800; color:var(--fg);}
    .uploadZone .uz-sub{font-size:11px; color:var(--mutedFg);}

    /* Progress bar */
    .uploadProgress{
      display:none;
      flex-direction:column;
      gap:8px;
      padding:12px;
      border-radius:var(--r-lg);
      border:var(--bw) solid var(--fg);
      background:var(--card);
      box-shadow:var(--shadow-pop);
    }
    .uploadProgress.show{ display:flex; }
    .uploadProgress .up-name{ font-size:12px; font-weight:800; white-space:nowrap; overflow:hidden; text-overflow:ellipsis; }
    .uploadProgress .up-meta{ font-size:11px; color:var(--mutedFg); }
    .progressTrack{
      height:10px;
      border-radius:var(--r-full);
      border:var(--bw) solid var(--fg);
      background:rgba(238,242,241,.90);
      overflow:hidden;
    }
    .progressFill{
      height:100%;
      width:0%;
      background: linear-gradient(90deg, var(--accent), var(--quaternary));
      border-radius:var(--r-full);
      transition: width .1s linear;
    }
    .uploadProgress .up-actions{ display:flex; gap:8px; justify-content:flex-end; }

    @media (prefers-reduced-motion: reduce){*{transition:none !important; animation:none !important}}
    @media (min-width: 860px){
      main{grid-template-columns: 1fr 1fr;}
      .span2{grid-column:1 / span 2;}
      header{padding:12px 4px 18px;}
      h1{font-size:18px;}
      .hrow{grid-template-columns:76px 1fr;}
      .pickItem .n{max-width:520px;}
    }
  </style>
</head>
<body>
  <div class="wrap">
    <header>
      <div class="brand">
        <div class="title">
          <div class="logo" aria-hidden="true"></div>
          <div>
            <h1>Audible Deterrent Control Panel</h1>
            <div class="sub">Schedule, audio library, and device time — designed for field setup on mobile.</div>
          </div>
        </div>
        <div class="chips">
          <span class="chip" title="Unit Serial Number">Serial: <span class="mono" id="serial">—</span></span>
          <span class="chip" id="statusChip" title="Connection status"><span class="dot disconnected" id="statusDot" aria-hidden="true"></span><span id="statusText">Connecting…</span></span>
          <span class="chip" title="Firmware version">Firmware: <span class="mono" id="firmware">—</span></span>
          <!-- USB drive status chip -->
          <span class="chip" id="usbChip" title="USB drive status"><span class="dot warning" id="usbDot" aria-hidden="true"></span><span id="usbText">USB: checking…</span></span>
        </div>
      </div>
      <div class="devicePill" id="openTime" role="button" tabindex="0" title="Device time (tap to set)">
        <span class="badge">Device</span>
        <span class="mono" id="deviceClock">--:--:--</span>
      </div>
    </header>

    <!-- USB warning banner (shown when drive not mounted) -->
    <div class="banner hidden" id="usbBanner" role="alert">
      <span class="dot warning" aria-hidden="true" style="flex:0 0 auto"></span>
      <span>USB drive not detected. Insert the drive and tap <strong>Retry</strong> — audio file list is unavailable until the drive is mounted.</span>
      <button class="btn secondary" id="retryUsb" style="white-space:nowrap; flex:0 0 auto">Retry</button>
    </div>

    <main>
      <!-- Weekly Schedule -->
      <section class="card span2">
        <div class="head">
          <h2>Weekly Schedule</h2>
          <div class="hint">Tap an hour to assign audio</div>
        </div>
        <div class="body">
          <div class="row" style="justify-content:space-between; align-items:flex-start">
            <div class="seg" role="tablist" aria-label="Choose day">
              <button role="tab" aria-selected="true"  data-day="0">Mon</button>
              <button role="tab" aria-selected="false" data-day="1">Tue</button>
              <button role="tab" aria-selected="false" data-day="2">Wed</button>
              <button role="tab" aria-selected="false" data-day="3">Thu</button>
              <button role="tab" aria-selected="false" data-day="4">Fri</button>
              <button role="tab" aria-selected="false" data-day="5">Sat</button>
              <button role="tab" aria-selected="false" data-day="6">Sun</button>
            </div>
            <div class="row" style="gap:8px">
              <button class="btn secondary" id="copyDay">Copy</button>
              <button class="btn secondary" id="pasteDay">Paste</button>
              <button class="btn primary"   id="saveSchedule">Save</button>
            </div>
          </div>
          <div class="hr"></div>
          <div class="scheduleWrap">
            <div class="dayMeta">
              <div class="left">
                <div class="d" id="dayTitle">Monday</div>
                <div class="s" id="daySummary">24 blocks • 0 active</div>
              </div>
              <div class="row" style="gap:8px">
                <button class="btn secondary" id="fillNone">Clear</button>
                <button class="btn secondary" id="fillDefault">Fill</button>
              </div>
            </div>
            <div class="hours" id="hours"></div>
            <div class="tiny">Each hour block can be silent or assigned to a file. Tap an hour to change its assignment.</div>
          </div>
        </div>
      </section>

      <!-- Audio Library -->
      <section class="card">
        <div class="head">
          <h2>Audio Library</h2>
          <div class="hint" id="libraryHint">Loading from device…</div>
        </div>
        <div class="body">

          <!-- Drop zone (hidden while upload is in progress) -->
          <div id="uploadZoneWrap">
            <div class="uploadZone" id="uploadZone" role="button" tabindex="0"
                 aria-label="Upload audio file — tap or drag and drop">
              <input type="file" id="filePick" accept=".wav,.mp3,.ogg,.flac,.aac" aria-hidden="true" />
              <div class="uz-icon" aria-hidden="true">🎵</div>
              <div class="uz-label">Tap to choose or drag &amp; drop</div>
              <div class="uz-sub">WAV · MP3 · OGG · FLAC · AAC — one file at a time</div>
            </div>
          </div>

          <!-- Progress (shown only during an active upload) -->
          <div class="uploadProgress" id="uploadProgress" role="status" aria-live="polite">
            <div class="up-name" id="upName">—</div>
            <div class="up-meta" id="upMeta">—</div>
            <div class="progressTrack">
              <div class="progressFill" id="progressFill"></div>
            </div>
            <div class="up-actions">
              <button class="btn bad" id="cancelUpload">Cancel</button>
            </div>
          </div>

          <div class="hr"></div>

          <table class="table" aria-label="Audio files">
            <thead>
              <tr>
                <th>File</th>
                <th style="width:18%">Size</th>
                <th style="width:18%">Actions</th>
              </tr>
            </thead>
            <tbody id="audioTbody">
              <tr><td colspan="3" class="tiny">Loading…</td></tr>
            </tbody>
          </table>
          <div class="tiny" style="margin-top:10px">
            Files are written directly to the USB drive. One upload at a time.
          </div>
        </div>
      </section>

      <!-- Do-Not-Play Dates -->
      <section class="card">
        <div class="head">
          <h2>Do-Not-Play Dates</h2>
          <div class="hint">Overrides weekly schedule</div>
        </div>
        <div class="body">
          <div class="row">
            <div style="flex:1; min-width:170px">
              <label for="dnpDate">Date</label>
              <input id="dnpDate" type="date" />
            </div>
            <div style="flex:2; min-width:220px">
              <label for="dnpNote">Note</label>
              <input id="dnpNote" type="text" placeholder="e.g., graduation ceremony" maxlength="40" />
            </div>
            <button class="btn primary" id="addDnp">Add</button>
          </div>
          <div class="hr"></div>
          <table class="table" aria-label="Do-not-play list">
            <thead>
              <tr>
                <th style="width:28%">Date</th>
                <th>Note</th>
                <th style="width:18%">Actions</th>
              </tr>
            </thead>
            <tbody id="dnpTbody"></tbody>
          </table>
          <div class="tiny" style="margin-top:10px">Playback is blocked for the full day on listed dates.</div>
        </div>
      </section>
    </main>
  </div>

  <!-- Bottom sheet modal -->
  <div class="backdrop" id="modal" role="dialog" aria-modal="true" aria-hidden="true">
    <div class="sheet">
      <div class="sheetHead">
        <div class="ttl" id="modalTitle">—</div>
        <div class="row" style="gap:8px">
          <button class="btn secondary" id="modalStop">Stop</button>
          <button class="btn primary"   id="modalPlay">Play</button>
          <button class="btn ghost"     id="modalClose">Close</button>
        </div>
      </div>
      <div class="sheetBody">
        <!-- Time set -->
        <div id="panelTime" class="stack" style="display:none">
          <div class="kvs">
            <div class="kv">
              <div>
                <div class="k">Current (device)</div>
                <div class="v mono" id="deviceNow">—</div>
              </div>
              <button class="btn ghost" id="syncLocal">Sync to phone</button>
            </div>
            <div class="kv">
              <div style="flex:1">
                <div class="k">Manual set</div>
                <div class="row" style="margin-top:10px">
                  <div style="flex:1; min-width:150px">
                    <label for="dtDate">Date</label>
                    <input id="dtDate" type="date" aria-label="Date" />
                  </div>
                  <div style="flex:1; min-width:150px">
                    <label for="dtTime">Time</label>
                    <input id="dtTime" type="time" step="1" aria-label="Time" />
                  </div>
                </div>
              </div>
              <button class="btn primary" id="applyTime">Apply</button>
            </div>
          </div>
          <div class="tiny" style="margin-top:10px">Firmware note: a real device would persist time in RTC and validate drift.</div>
        </div>
        <!-- Audio test -->
        <div id="panelTest" class="stack" style="display:none">
          <div class="kvs">
            <div class="kv"><div class="k">Name</div><div class="v mono" id="mName">—</div></div>
            <div class="kv"><div class="k">Size</div><div class="v mono" id="mSize">—</div></div>
          </div>
          <div class="hr"></div>
          <div class="tiny">Preview requires a blobUrl (browser upload). Files loaded from USB cannot be previewed in-browser.</div>
          <div style="margin-top:10px"><audio id="player" controls style="width:100%"></audio></div>
        </div>
        <!-- Hour assignment -->
        <div id="panelAssign" class="stack" style="display:none">
          <div class="kvs">
            <div class="kv">
              <div>
                <div class="k">Hour</div>
                <div class="v"><span class="mono" id="aDay">—</span> • <span class="mono" id="aHour">—</span></div>
              </div>
              <span class="tag" id="aCurrent">—</span>
            </div>
          </div>
          <div class="hr"></div>
          <div class="pickList" id="pickList"></div>
          <div class="tiny" style="margin-top:10px">Pick an audio file for this hour, or set it to silent.</div>
        </div>
      </div>
    </div>
  </div>

  <div class="toast" id="toast" role="status" aria-live="polite">
    <span class="chip" style="padding:0; border:none; box-shadow:none; background:transparent">
      <span class="dot" aria-hidden="true" style="background:rgba(47,111,85,.28)"></span>
    </span>
    <span id="toastMsg">Saved</span>
  </div>

  <script>
    "use strict";

    // ── Constants ──────────────────────────────────────────
    const DAYS = ["Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"];

    // ── State ─────────────────────────────────────────────
    //
    // audioFiles entries: { id, name, bytes, blobUrl }
    //   id      — browser-only stable key for DOM/modal references
    //   name    — actual filename on USB (e.g. "hawk.wav")
    //   blobUrl — set only for browser-uploaded files; null for USB-only
    //
    // schedule[day][hour] = filename string (e.g. "hawk.wav") or "" for silent.
    // Filenames are stored directly — no IDs — so they round-trip cleanly
    // through config.json without any translation layer.
    //
    // doNotPlay entries: { id, date, note }
    //   id is browser-only (for DOM keying); not persisted to config.
    //
    let audioFiles = [];
    let schedule   = Array.from({length:7}, () => Array.from({length:24}, () => ""));
    let doNotPlay  = [];
    let copiedDay  = null;
    let activeDay  = 0;

    // modal state
    let modalMode    = null;
    let modalAudioId = null;
    let assignHour   = null;

    // ── Helpers ───────────────────────────────────────────
    function cryptoRandomId() {
      try {
        return ([1e7]+-1e3+-4e3+-8e3+-1e11).replace(/[018]/g, c =>
          (c ^ crypto.getRandomValues(new Uint8Array(1))[0] & 15 >> c / 4).toString(16)
        );
      } catch(e) {
        return "id_" + Math.random().toString(16).slice(2) + "_" + Date.now().toString(16);
      }
    }

    function fmtBytes(n) {
      if (!Number.isFinite(n)) return "—";
      const u = ["B","KB","MB","GB"]; let i=0; let x=n;
      while (x>=1024 && i<u.length-1) { x/=1024; i++; }
      return (i===0 ? x.toFixed(0) : x.toFixed(1)) + " " + u[i];
    }

    function hourLabel(h) { return String(h).padStart(2,'0') + ":00"; }

    function nowIsoLocal() {
      const d = new Date();
      const tz = -d.getTimezoneOffset();
      const sign = tz>=0 ? "+" : "-";
      const hh = String(Math.floor(Math.abs(tz)/60)).padStart(2,'0');
      const mm = String(Math.abs(tz)%60).padStart(2,'0');
      return d.getFullYear()+"-"+String(d.getMonth()+1).padStart(2,'0')+"-"+
             String(d.getDate()).padStart(2,'0')+"T"+
             String(d.getHours()).padStart(2,'0')+":"+
             String(d.getMinutes()).padStart(2,'0')+":"+
             String(d.getSeconds()).padStart(2,'0')+sign+hh+":"+mm;
    }

    function isoDateLocalOffset(days) {
      const d = new Date();
      d.setDate(d.getDate() + days);
      return d.toISOString().slice(0,10);
    }

    function toast(msg) {
      const el = document.getElementById('toast');
      const t  = document.getElementById('toastMsg');
      if (!el || !t) return;
      t.textContent = msg;
      el.classList.add('show');
      clearTimeout(toast._t);
      toast._t = setTimeout(() => el.classList.remove('show'), 1800);
    }

    function escapeHtml(s) {
      return String(s)
        .replaceAll('&','&amp;').replaceAll('<','&lt;').replaceAll('>','&gt;')
        .replaceAll('"','&quot;').replaceAll("'",'&#039;');
    }

    // ── Audio file lookup helpers ──────────────────────────
    // Two separate lookups: by filename (the persistent key used in
    // schedule slots) and by browser id (used only in the Test modal).
    function findAudioByName(name) { return audioFiles.find(a => a.name === name) || null; }
    function findAudioById(id)     { return audioFiles.find(a => a.id   === id)   || null; }

    // Returns true if a slot has a filename but that file isn't on USB
    function isMissing(filename) { return !!filename && !findAudioByName(filename); }

    // Schedule slot display helpers — work with filenames, not IDs
    function fileLabel(filename) {
      if (!filename) return "(silent)";
      return filename;
    }

    function fileMeta(filename) {
      if (!filename) return "No playback";
      const a = findAudioByName(filename);
      if (!a) return "⚠ File missing from USB";
      return fmtBytes(a.bytes);
    }

    function swatchClass(i) { return ["v","p","y","g"][i % 4]; }

    // ── Device init sequence ──────────────────────────────
    //
    // Three-step chain on page load:
    // 1. GET /settings  → serial, firmware, usbMounted, time
    // 2. GET /files     → audio file list
    // 3. GET /config    → schedule + doNotPlay
    //    (must come after /files so missing-file detection works)
    //
    async function initDevice() {
      // Step 1: settings
      try {
        const res  = await fetch("/settings");
        const data = await res.json();

        const setEl = (id, val) => {
          const el = document.getElementById(id);
          if (!el) return;
          if (el.tagName === "INPUT" || el.tagName === "SELECT") el.value = val;
          else el.textContent = val;
        };

        setEl("serial",   data.serial   ?? "—");
        setEl("firmware", data.firmware ?? "—");

        updateUsbStatus(data.usbMounted === true);

        // If device sent current time, pre-fill the manual time fields
        if (data.time) {
          const [datePart, timePart] = data.time.split("T");
          if (datePart) document.getElementById('dtDate').value = datePart;
          if (timePart) document.getElementById('dtTime').value = timePart;
        }
      } catch(e) {
        console.warn("[initDevice] /settings failed:", e);
        // Connection check will update the status chip separately
      }

      // Step 2: file list
      await refreshFileList();

      // Step 3: persisted config (schedule + do-not-play)
      await loadConfig();
    }

    // ── USB status helpers ────────────────────────────────
    function updateUsbStatus(mounted) {
      const dot    = document.getElementById('usbDot');
      const text   = document.getElementById('usbText');
      const banner = document.getElementById('usbBanner');
      if (!dot || !text || !banner) return;

      if (mounted) {
        dot.className    = "dot connected";
        text.textContent = "USB: Ready";
        banner.classList.add("hidden");
      } else {
        dot.className    = "dot warning";
        text.textContent = "USB: No drive";
        banner.classList.remove("hidden");
      }
    }

    // ── File list fetch ───────────────────────────────────
    //
    // Fetches /files, merges result into audioFiles array.
    // Keeps any browser-uploaded entries (blobUrl != null) so
    // they aren't lost on refresh.
    //
    async function refreshFileList() {
      const hint = document.getElementById('libraryHint');
      if (hint) hint.textContent = "Loading from device…";

      try {
        const res = await fetch("/files");

        if (res.status === 503) {
          // USB not mounted
          updateUsbStatus(false);
          audioFiles = audioFiles.filter(a => a.blobUrl); // keep uploads only
          renderAudioTable();
          if (hint) hint.textContent = "USB drive not mounted";
          return;
        }

        if (!res.ok) throw new Error("HTTP " + res.status);

        const files = await res.json(); // [ { name, bytes }, … ]

        // Build new list:
        // - Start with USB files from device (no blobUrl)
        // - Append any browser-uploaded files that aren't already there
        const usbFiles = files.map(f => {
          // Preserve existing id if we already have this filename
          const existing = audioFiles.find(a => a.name === f.name);
          return {
            id:      existing ? existing.id : cryptoRandomId(),
            name:    f.name,
            bytes:   f.bytes,
            blobUrl: existing ? existing.blobUrl : null
          };
        });

        const browserOnly = audioFiles.filter(a =>
          a.blobUrl && !usbFiles.some(u => u.name === a.name)
        );

        audioFiles = [...usbFiles, ...browserOnly];
        updateUsbStatus(true);
        renderAudioTable();
        renderHours(); // refresh schedule display in case file names changed

        if (hint) hint.textContent = `${usbFiles.length} file${usbFiles.length !== 1 ? "s" : ""} on device`;

      } catch(e) {
        console.warn("[refreshFileList] failed:", e);
        if (hint) hint.textContent = "Could not load file list";
      }
    }

    // ── Config load ───────────────────────────────────────
    // Fetches /config and populates schedule + doNotPlay.
    // Must run after refreshFileList() so isMissing() has data.
    async function loadConfig() {
      try {
        const res = await fetch("/config");
        if (!res.ok) { console.warn("[loadConfig] HTTP", res.status); return; }
        const data = await res.json();

        // Schedule: 7 days × 24 hours of filename strings.
        // Pad with "" if device returned fewer entries (e.g. first boot).
        if (Array.isArray(data.schedule)) {
          for (let d = 0; d < 7; d++) {
            const day = Array.isArray(data.schedule[d]) ? data.schedule[d] : [];
            for (let h = 0; h < 24; h++) {
              schedule[d][h] = (typeof day[h] === "string") ? day[h] : "";
            }
          }
        }

        // Do-not-play: regenerate browser-side ids (not persisted).
        if (Array.isArray(data.doNotPlay)) {
          doNotPlay = data.doNotPlay
            .filter(x => x.date)
            .map(x => ({ id: cryptoRandomId(), date: x.date, note: x.note ?? "" }));
        }

        renderHours();
        renderDnp();

        // Warn if any scheduled slots reference files not currently on USB
        const missingNames = new Set();
        for (let d = 0; d < 7; d++)
          for (let h = 0; h < 24; h++) {
            const fn = schedule[d][h];
            if (fn && isMissing(fn)) missingNames.add(fn);
          }
        if (missingNames.size > 0)
          toast(`⚠ ${missingNames.size} scheduled file${missingNames.size > 1 ? "s" : ""} missing from USB`);

      } catch(e) {
        console.warn("[loadConfig] failed:", e);
        toast("Could not load saved schedule");
      }
    }

    // ── Config save ───────────────────────────────────────
    // Serializes current schedule + doNotPlay and POSTs to /config.
    // Triggered only when the user taps Save.
    async function saveConfig() {
      const btn = document.getElementById('saveSchedule');
      if (btn) { btn.textContent = "Saving…"; btn.disabled = true; }

      // Build payload — filenames only, no browser-side IDs.
      // doNotPlay drops the browser id since it isn't meaningful on device.
      const payload = {
        schedule:  schedule.map(day => [...day]),
        doNotPlay: doNotPlay.map(({ date, note }) => ({ date, note }))
      };

      try {
        const res = await fetch("/config", {
          method:  "POST",
          headers: { "Content-Type": "application/json" },
          body:    JSON.stringify(payload)
        });
        if (!res.ok) throw new Error((await res.text().catch(() => "")) || "HTTP " + res.status);
        toast("Schedule saved to device ✓");
      } catch(e) {
        console.error("[saveConfig]", e);
        toast("Save failed: " + e.message);
      } finally {
        if (btn) { btn.textContent = "Save"; btn.disabled = false; }
      }
    }

    // ── Renderers ─────────────────────────────────────────
    function renderDeviceClock() {
      const elClock = document.getElementById('deviceClock');
      const elNow   = document.getElementById('deviceNow');
      if (!elClock || !elNow) return;
      const d  = new Date();
      const hh = String(d.getHours()).padStart(2,'0');
      const mm = String(d.getMinutes()).padStart(2,'0');
      const ss = String(d.getSeconds()).padStart(2,'0');
      elClock.textContent = `${hh}:${mm}:${ss}`;
      elNow.textContent   = nowIsoLocal();
    }

    function renderAudioTable() {
      const tb = document.getElementById('audioTbody');
      if (!tb) return;
      if (!audioFiles.length) {
        tb.innerHTML = `<tr><td colspan="3" class="tiny">No audio files found on USB drive.</td></tr>`;
        return;
      }
      tb.innerHTML = audioFiles.map((a, idx) => {
        const sw = swatchClass(idx);
        const canPreview = !!a.blobUrl;
        return `
          <tr>
            <td>
              <div class="row" style="gap:10px">
                <span class="swatch ${sw}" aria-hidden="true"></span>
                <span class="mono" style="font-weight:800">${escapeHtml(a.name)}</span>
              </div>
            </td>
            <td class="mono">${escapeHtml(fmtBytes(a.bytes))}</td>
            <td>
              <div class="row" style="gap:8px">
                <button class="btn secondary" data-act="test" data-id="${escapeHtml(a.id)}"
                  ${canPreview ? "" : 'title="No preview — file is on USB only"'}
                >Test</button>
              </div>
            </td>
          </tr>
        `;
      }).join("");
    }

    function renderDnp() {
      const tb = document.getElementById('dnpTbody');
      if (!tb) return;
      if (!doNotPlay.length) {
        tb.innerHTML = `<tr><td colspan="3" class="tiny">No special dates added.</td></tr>`;
        return;
      }
      const sorted = [...doNotPlay].sort((a,b) => a.date.localeCompare(b.date));
      tb.innerHTML = sorted.map(x => `
        <tr>
          <td class="mono" style="font-weight:800">${escapeHtml(x.date)}</td>
          <td>${escapeHtml(x.note || "(no note)")}</td>
          <td><button class="btn bad" data-act="dnpdel" data-id="${escapeHtml(x.id)}">Remove</button></td>
        </tr>
      `).join("");
    }

    function renderDayHeader() {
      const t = document.getElementById('dayTitle');
      const s = document.getElementById('daySummary');
      if (!t || !s) return;
      t.textContent = DAYS[activeDay];
      const act     = schedule[activeDay].filter(v => v !== "").length;
      const missing = schedule[activeDay].filter(v => isMissing(v)).length;
      s.textContent = `24 blocks • ${act} active${missing ? ` • ${missing} missing` : ""}`;
    }

    function renderHours() {
      const box = document.getElementById('hours');
      if (!box) return;
      const rows = [];
      for (let h=0; h<24; h++) {
        const filename = schedule[activeDay][h];
        const lbl      = fileLabel(filename);
        const meta     = fileMeta(filename);
        const isSilent = !filename;
        const missing  = isMissing(filename);
        // Tag reflects the slot state: silent, missing file, or assigned
        const tagClass = isSilent ? "silent" : missing ? "missing" : "";
        const tagText  = isSilent ? "Silent"  : missing ? "Missing" : "Edit";
        rows.push(`
          <div class="hrow">
            <div class="t">${hourLabel(h)}</div>
            <div class="assign" role="button" tabindex="0" data-act="assign" data-hour="${h}" aria-label="Assign audio for ${hourLabel(h)}">
              <div class="left">
                <div class="name">${escapeHtml(lbl)}</div>
                <div class="meta">${escapeHtml(meta)}</div>
              </div>
              <span class="tag ${tagClass}">${tagText}</span>
            </div>
          </div>
        `);
      }
      box.innerHTML = rows.join("");
      renderDayHeader();
    }

    // ── Modal ─────────────────────────────────────────────
    function showModal(mode) {
      modalMode = mode;
      document.getElementById('panelTest').style.display   = (mode === 'test')   ? 'block' : 'none';
      document.getElementById('panelAssign').style.display = (mode === 'assign') ? 'block' : 'none';
      document.getElementById('panelTime').style.display   = (mode === 'time')   ? 'block' : 'none';
      const audioCtrl = mode === 'test' || mode === 'assign';
      document.getElementById('modalPlay').style.display   = audioCtrl ? 'inline-block' : 'none';
      document.getElementById('modalStop').style.display   = audioCtrl ? 'inline-block' : 'none';
      const modal = document.getElementById('modal');
      modal.classList.add('show');
      modal.setAttribute('aria-hidden','false');
    }

    function closeModal() {
      const player = document.getElementById('player');
      if (player) { try { player.pause(); player.currentTime = 0; } catch(_){} }
      const modal = document.getElementById('modal');
      modal.classList.remove('show');
      modal.setAttribute('aria-hidden','true');
      modalMode = modalAudioId = assignHour = null;
    }

    // Player uses browser-side id to find blobUrl (only browser-uploaded files have one)
    function setPlayerFromAudioId(id) {
      const player = document.getElementById('player');
      if (!player) return;
      try { player.pause(); player.currentTime = 0; } catch(_) {}
      const a = id ? findAudioById(id) : null;
      if (a && a.blobUrl) { player.src = a.blobUrl; player.load(); }
      else { player.removeAttribute('src'); player.load(); }
    }

    function openModalForAudioTest(id) {
      const a = findAudioById(id);
      if (!a) { toast('File not found'); return; }
      modalAudioId = id;
      document.getElementById('modalTitle').textContent = "Test Audio";
      document.getElementById('mName').textContent = a.name;
      document.getElementById('mSize').textContent = fmtBytes(a.bytes);
      setPlayerFromAudioId(id);
      showModal('test');
    }

    function openModalForTime() {
      document.getElementById('modalTitle').textContent = "Device Time";
      document.getElementById('deviceNow').textContent  = nowIsoLocal();
      showModal('time');
    }

    function openModalForHourAssign(day, hour) {
      assignHour = { day, hour };
      const currentFilename = schedule[day][hour];
      document.getElementById('modalTitle').textContent = "Assign Hour";
      document.getElementById('aDay').textContent  = DAYS[day].slice(0,3);
      document.getElementById('aHour').textContent = hourLabel(hour);
      const aCur    = document.getElementById('aCurrent');
      const missing = isMissing(currentFilename);
      aCur.textContent = !currentFilename ? "Silent" : missing ? "Missing" : "Set";
      aCur.className   = "tag" + (!currentFilename ? " silent" : missing ? " missing" : "");
      renderPickList(currentFilename);
      // Player: look up by name to get the id, then pass to setPlayer
      const a = currentFilename ? findAudioByName(currentFilename) : null;
      setPlayerFromAudioId(a ? a.id : null);
      showModal('assign');
    }

    function renderPickList(selectedFilename) {
      const list = document.getElementById('pickList');
      if (!list) return;
      // Silent option always first
      const items = [`
        <div class="pickItem" role="button" tabindex="0" data-pick="" aria-label="Set silent">
          <div><div class="n">(silent)</div><div class="m">No playback</div></div>
          <div class="right"><span class="tag silent">Silent</span></div>
        </div>
      `];
      // Available files from USB — data-pick stores filename directly
      audioFiles.forEach((a, idx) => {
        const sw    = swatchClass(idx);
        const isSel = a.name === selectedFilename;
        items.push(`
          <div class="pickItem" role="button" tabindex="0"
               data-pick="${escapeHtml(a.name)}"
               aria-label="Pick ${escapeHtml(a.name)}">
            <div>
              <div class="n">${escapeHtml(a.name)}</div>
              <div class="m">${escapeHtml(fmtBytes(a.bytes))}</div>
            </div>
            <div class="right">
              <span class="swatch ${sw}" aria-hidden="true"></span>
              <span class="tag">${isSel ? "Selected" : "Pick"}</span>
            </div>
          </div>
        `);
      });
      // If the current slot references a missing file, show it as a
      // grayed-out warning entry so the user can see what was there
      if (selectedFilename && isMissing(selectedFilename)) {
        items.push(`
          <div class="pickItem" style="opacity:.55; cursor:not-allowed"
               aria-label="Missing file: ${escapeHtml(selectedFilename)}">
            <div>
              <div class="n">⚠ ${escapeHtml(selectedFilename)}</div>
              <div class="m">Not found on USB drive</div>
            </div>
            <div class="right"><span class="tag missing">Missing</span></div>
          </div>
        `);
      }
      list.innerHTML = items.join("");
    }

    // ── Connection check ──────────────────────────────────
    async function checkConnection() {
      const dot  = document.getElementById('statusDot');
      const text = document.getElementById('statusText');
      const ctrl = new AbortController();
      const timer = setTimeout(() => ctrl.abort(), 2000);
      try {
        const res = await fetch("/ping?t=" + Date.now(), { signal: ctrl.signal });
        clearTimeout(timer);
        if (!res.ok) throw new Error();
        dot.className    = "dot connected";
        text.textContent = "Connected";
      } catch(_) {
        clearTimeout(timer);
        dot.className    = "dot disconnected";
        text.textContent = "Disconnected";
      }
    }

    // ── Time sync ─────────────────────────────────────────
    async function syncTime() {
      const now = new Date();
      // Pre-fill manual fields
      document.getElementById('dtDate').value =
        now.getFullYear() + "-" +
        String(now.getMonth()+1).padStart(2,'0') + "-" +
        String(now.getDate()).padStart(2,'0');
      document.getElementById('dtTime').value =
        String(now.getHours()).padStart(2,'0') + ":" +
        String(now.getMinutes()).padStart(2,'0') + ":" +
        String(now.getSeconds()).padStart(2,'0');

      try {
        await fetch("/synctime", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({
            year: now.getFullYear(), month: now.getMonth()+1,
            day: now.getDate(), hour: now.getHours(),
            minute: now.getMinutes(), second: now.getSeconds()
          })
        });
        toast("Time synced to device");
      } catch(_) {
        toast("Sync failed (offline?)");
      }
    }

    // ── Wiring ────────────────────────────────────────────
    window.addEventListener('DOMContentLoaded', () => {

      // Day tabs
      document.querySelectorAll('.seg button').forEach(btn => {
        btn.addEventListener('click', () => {
          activeDay = Number(btn.dataset.day);
          document.querySelectorAll('.seg button').forEach(b => b.setAttribute('aria-selected','false'));
          btn.setAttribute('aria-selected','true');
          renderHours();
        });
      });

      // Hour assign
      document.getElementById('hours').addEventListener('click', e => {
        const a = e.target.closest('.assign');
        if (!a) return;
        openModalForHourAssign(activeDay, Number(a.dataset.hour));
      });
      document.getElementById('hours').addEventListener('keydown', e => {
        const a = e.target.closest('.assign');
        if (!a) return;
        if (e.key === 'Enter' || e.key === ' ') {
          e.preventDefault();
          openModalForHourAssign(activeDay, Number(a.dataset.hour));
        }
      });

      // Pick list — data-pick now contains the filename (or "" for silent)
      document.getElementById('pickList').addEventListener('click', e => {
        const item = e.target.closest('.pickItem[data-pick]');
        if (!item || !assignHour) return;
        const pickedName = item.dataset.pick; // filename or "" for silent
        schedule[assignHour.day][assignHour.hour] = pickedName;
        renderHours();
        renderPickList(pickedName);
        // Update player — look up by name to get id for blobUrl
        const a = pickedName ? findAudioByName(pickedName) : null;
        setPlayerFromAudioId(a ? a.id : null);
        toast(pickedName ? 'Hour updated' : 'Hour set to silent');
      });

      // Copy/paste day
      document.getElementById('copyDay').addEventListener('click', () => {
        copiedDay = [...schedule[activeDay]];
        toast(`Copied ${DAYS[activeDay]}`);
      });
      document.getElementById('pasteDay').addEventListener('click', () => {
        if (!copiedDay) { toast('Nothing copied'); return; }
        schedule[activeDay] = [...copiedDay];
        renderHours();
        toast(`Pasted into ${DAYS[activeDay]}`);
      });

      // Fill helpers
      document.getElementById('fillNone').addEventListener('click', () => {
        schedule[activeDay] = schedule[activeDay].map(() => "");
        renderHours();
        toast('Cleared day');
      });
      document.getElementById('fillDefault').addEventListener('click', () => {
        // Fill uses filename directly (not id)
        const firstName = audioFiles[0]?.name || "";
        const arr       = Array.from({length:24}, () => "");
        for (let h=6; h<=19; h++) arr[h] = firstName;
        schedule[activeDay] = arr;
        renderHours();
        toast('Filled day');
      });

      // Save button — POSTs full schedule + doNotPlay to /config
      document.getElementById('saveSchedule').addEventListener('click', saveConfig);

      // Audio table actions
      document.getElementById('audioTbody').addEventListener('click', e => {
        const btn = e.target.closest('button');
        if (!btn) return;
        const id  = btn.dataset.id;
        const act = btn.dataset.act;
        if (act === 'test') openModalForAudioTest(id);
      });

      // ── Upload ────────────────────────────────────────────────
      //
      // Uses XMLHttpRequest (not fetch) so we get xhr.upload.onprogress.
      // Files are sent as multipart/form-data — the Arduino parses the
      // boundary and streams bytes directly to the USB filesystem.
      //
      let activeXhr = null;  // track in-flight upload for cancel

      function fmtBytesTransfer(loaded, total) {
        return fmtBytes(loaded) + " / " + fmtBytes(total);
      }

      function startUpload(file) {
        if (!file) return;

        // Guard: only audio extensions
        const allowed = ['.wav','.mp3','.ogg','.flac','.aac'];
        const ext = file.name.substring(file.name.lastIndexOf('.')).toLowerCase();
        if (!allowed.includes(ext)) {
          toast('Unsupported file type');
          return;
        }

        // Guard: USB must be mounted
        const usbDot = document.getElementById('usbDot');
        if (usbDot && usbDot.classList.contains('warning')) {
          toast('USB drive not ready');
          return;
        }

        // Show progress, hide drop zone
        document.getElementById('uploadZoneWrap').style.display  = 'none';
        document.getElementById('uploadProgress').classList.add('show');
        document.getElementById('upName').textContent = file.name;
        document.getElementById('upMeta').textContent = '0 B / ' + fmtBytes(file.size);
        document.getElementById('progressFill').style.width = '0%';

        const formData = new FormData();
        formData.append('file', file, file.name);

        const xhr = new XMLHttpRequest();
        activeXhr = xhr;

        // Progress
        xhr.upload.addEventListener('progress', e => {
          if (!e.lengthComputable) return;
          const pct = Math.round((e.loaded / e.total) * 100);
          document.getElementById('progressFill').style.width = pct + '%';
          document.getElementById('upMeta').textContent = fmtBytesTransfer(e.loaded, e.total) + ' — ' + pct + '%';
        });

        // Done
        xhr.addEventListener('load', async () => {
          activeXhr = null;
          resetUploadUI();

          if (xhr.status === 200) {
            toast('Upload complete — refreshing file list…');
            await refreshFileList();
          } else {
            // Try to surface the error message the Arduino sent
            const msg = xhr.responseText ? xhr.responseText.trim() : ('Error ' + xhr.status);
            toast('Upload failed: ' + msg);
          }
        });

        // Network error / timeout
        xhr.addEventListener('error', () => {
          activeXhr = null;
          resetUploadUI();
          toast('Upload failed — network error');
        });
        xhr.addEventListener('timeout', () => {
          activeXhr = null;
          resetUploadUI();
          toast('Upload timed out');
        });

        // Abort (cancel button)
        xhr.addEventListener('abort', () => {
          activeXhr = null;
          resetUploadUI();
          toast('Upload cancelled');
        });

        // 120 s timeout — large files over WiFi AP take time
        xhr.timeout = 120000;
        xhr.open('POST', '/upload');
        xhr.send(formData);
      }

      function resetUploadUI() {
        document.getElementById('uploadZoneWrap').style.display  = '';
        document.getElementById('uploadProgress').classList.remove('show');
        document.getElementById('progressFill').style.width = '0%';
        // Reset the file input so the same file can be re-selected
        document.getElementById('filePick').value = '';
      }

      // File input change
      document.getElementById('filePick').addEventListener('change', e => {
        const file = e.target.files[0];
        if (file) startUpload(file);
      });

      // Drag-and-drop on the upload zone
      const uploadZone = document.getElementById('uploadZone');
      uploadZone.addEventListener('dragover', e => {
        e.preventDefault();
        uploadZone.classList.add('drag');
      });
      uploadZone.addEventListener('dragleave', () => uploadZone.classList.remove('drag'));
      uploadZone.addEventListener('drop', e => {
        e.preventDefault();
        uploadZone.classList.remove('drag');
        const file = e.dataTransfer.files[0];
        if (file) startUpload(file);
      });

      // Cancel button
      document.getElementById('cancelUpload').addEventListener('click', () => {
        if (activeXhr) activeXhr.abort();
      });

      // Do-not-play
      document.getElementById('addDnp').addEventListener('click', () => {
        const date = document.getElementById('dnpDate').value;
        const note = document.getElementById('dnpNote').value.trim();
        if (!date) { toast('Pick a date'); return; }
        if (doNotPlay.some(x => x.date === date)) { toast('Date already added'); return; }
        doNotPlay.push({ id: cryptoRandomId(), date, note });
        document.getElementById('dnpDate').value = "";
        document.getElementById('dnpNote').value = "";
        renderDnp();
        toast('Added date');
      });
      document.getElementById('dnpTbody').addEventListener('click', e => {
        const btn = e.target.closest('button');
        if (!btn || btn.dataset.act !== 'dnpdel') return;
        doNotPlay = doNotPlay.filter(x => x.id !== btn.dataset.id);
        renderDnp();
        toast('Removed');
      });

      // Retry USB
      document.getElementById('retryUsb').addEventListener('click', async () => {
        toast('Checking USB drive…');
        await refreshFileList();
      });

      // Time controls
      document.getElementById('syncLocal').addEventListener('click', syncTime);
      document.getElementById('applyTime').addEventListener('click', () => {
        const date = document.getElementById('dtDate').value;
        const time = document.getElementById('dtTime').value;
        if (!date || !time) { toast('Set date and time'); return; }
        toast('Time applied');
      });

      // Open time modal
      const openTime = document.getElementById('openTime');
      openTime.addEventListener('click', openModalForTime);
      openTime.addEventListener('keydown', e => {
        if (e.key === 'Enter' || e.key === ' ') { e.preventDefault(); openModalForTime(); }
      });

      // Modal controls
      document.getElementById('modalClose').addEventListener('click', closeModal);
      document.getElementById('modal').addEventListener('click', e => {
        if (e.target.id === 'modal') closeModal();
      });
      document.getElementById('modalPlay').addEventListener('click', () => {
        const player = document.getElementById('player');
        if (!player || !player.src) { toast('No preview (USB-only file)'); return; }
        player.play().catch(() => toast('Playback blocked'));
      });
      document.getElementById('modalStop').addEventListener('click', () => {
        const player = document.getElementById('player');
        if (player) { player.pause(); player.currentTime = 0; }
      });

      // Initial render (empty state while loading)
      renderHours();
      renderDnp();

      // Device init
      initDevice();

      // Clock tick
      renderDeviceClock();
      setInterval(renderDeviceClock, 1000);
    });

    // Connection polling
    checkConnection();
    setInterval(checkConnection, 3000);
  </script>
</body>
</html>
)rawliteral";