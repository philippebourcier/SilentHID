#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// Network Configuration
// ============================================================================

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
#define NET_MAC { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }

// Useless if you enable DHCP
#define NET_IP IPAddress(10, 200, 200, 200)
#define NET_PORT 443

// ============================================================================
// Authentication
// ============================================================================
#define WS_USERNAME "admin"
#define WS_PASSWORD "secret"

// ============================================================================
// Timing Configuration
// ============================================================================
#define MOUSE_THROTTLE_MS 20
#define HID_MIN_INTERVAL_MS 5
#define HID_READY_TIMEOUT_MS 100
#define WS_FRAME_TIMEOUT_MS 5000
#define CLIENT_TIMEOUT_MS 5000
#define TLS_FLUSH_TIMEOUT_MS 100

// ============================================================================
// HTML page
// ============================================================================
static const char html_page[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>HID Control</title>
<style>
*{box-sizing:border-box;font-family:system-ui,sans-serif;margin:0;padding:0}
body{background:#1a1a2e;color:#eee;padding:10px;height:100vh;display:flex;flex-direction:column}
h1{color:#0df;font-size:1.2em;margin-bottom:3px}
.head{display:flex;justify-content:space-between;align-items:center;margin-bottom:8px}
.sub{color:#666;font-size:.75em}
.row{display:flex;gap:8px;flex-wrap:wrap}
.panel{background:#16213e;border-radius:6px;padding:10px;margin-bottom:8px}
.panel h2{font-size:.85em;color:#0df;border-bottom:1px solid #333;padding-bottom:4px;margin-bottom:6px}
label{font-size:.7em;color:#888;display:block;margin-bottom:2px}
input,select{padding:5px 7px;border:1px solid #333;border-radius:3px;background:#0f0f23;color:#eee;font-size:11px}
input:focus{outline:none;border-color:#0df}
button{padding:5px 10px;border:none;border-radius:3px;cursor:pointer;font-size:11px}
.btn-p{background:#0df;color:#000}
.btn-s{background:#444;color:#fff}
.btn-k{min-width:36px;height:36px;background:#2d3748;color:#fff;border:1px solid #444;font-size:10px}
.btn-k.w{min-width:65px}
.status{padding:2px 8px;border-radius:10px;font-size:.7em}
.status.d{background:#f45;color:#fff}.status.c{background:#2e5;color:#000}.status.g{background:#fa0;color:#000}
#log{background:#0f0f23;border:1px solid #333;border-radius:3px;padding:5px;height:120px;overflow-y:auto;font-family:monospace;font-size:10px}
.tx{color:#0df}.rx{color:#2e5}.err{color:#f45}.info{color:#666}
.mp{width:100%;height:100%;background:#0f0f23;border:2px solid #333;border-radius:6px;cursor:crosshair;touch-action:none;display:flex;align-items:center;justify-content:center;color:#333;font-size:12px;min-height:200px}
.mp:hover{border-color:#0df}
.mb{display:flex;gap:4px;margin-top:6px}
.mb button{flex:1;height:45px;background:#2d3748;border:1px solid #444;border-radius:3px;color:#fff;font-size:12px}
.mb button.a{background:#0df;color:#000}
.acc{border:1px solid #333;border-radius:4px;margin-bottom:4px}
.acc-h{background:#2d3748;padding:5px 8px;cursor:pointer;font-size:.75em;display:flex;justify-content:space-between}
.acc-h::after{content:'▼';font-size:.55em;transition:transform .2s}
.acc.open .acc-h::after{transform:rotate(180deg)}
.acc-c{max-height:0;overflow:hidden;transition:max-height .3s;padding:0 6px}
.acc.open .acc-c{max-height:200px;padding:6px}
.ki{width:100%;padding:7px;font-size:12px;background:#0f0f23;border:1px solid #333;border-radius:3px;color:#fff;margin-bottom:5px}
.main{display:flex;gap:8px;flex:1;min-height:0}
.kb-panel{width:100%;max-width:762px;flex-shrink:0}
.mouse-panel{flex:1;display:flex;flex-direction:column;min-width:300px}
.mouse-panel .mp-wrap{flex:1;display:flex;flex-direction:column}
.mouse-panel .mp{flex:1}
.bottom{display:flex;gap:8px}
.log-panel{flex:1}
.raw-panel{width:320px;flex-shrink:0}
@media(max-width:1100px){.main{flex-direction:column}.kb-panel{max-width:100%}.bottom{flex-direction:column}.raw-panel{width:100%}}
</style>
</head>
<body>
<div class="head">
<div><h1>🎮 HID Control</h1><span class="sub">Keyboard & Mouse over WebSocket</span></div>
<div>Status: <span id="st" class="status g">...</span></div>
</div>
<div class="main">
<div class="panel kb-panel">
<h2>⌨️ Keyboard</h2>
<input type="text" id="ti" class="ki" placeholder="Type and press Enter..." onkeydown="hti(event)">
<div class="acc open" id="a1"><div class="acc-h" onclick="tacc('a1')">Function Keys</div><div class="acc-c"><div class="row">
<button class="btn-k" onclick="sk(58)">F1</button><button class="btn-k" onclick="sk(59)">F2</button><button class="btn-k" onclick="sk(60)">F3</button><button class="btn-k" onclick="sk(61)">F4</button><button class="btn-k" onclick="sk(62)">F5</button><button class="btn-k" onclick="sk(63)">F6</button><button class="btn-k" onclick="sk(64)">F7</button><button class="btn-k" onclick="sk(65)">F8</button><button class="btn-k" onclick="sk(66)">F9</button><button class="btn-k" onclick="sk(67)">F10</button><button class="btn-k" onclick="sk(68)">F11</button><button class="btn-k" onclick="sk(69)">F12</button>
</div></div></div>
<div class="acc open" id="a2"><div class="acc-h" onclick="tacc('a2')">Special Keys</div><div class="acc-c"><div class="row">
<button class="btn-k w" onclick="sk(41)">Esc</button><button class="btn-k w" onclick="sk(43)">Tab</button><button class="btn-k w" onclick="sk(40)">Enter</button><button class="btn-k w" onclick="sk(42)">Bksp</button><button class="btn-k w" onclick="sk(76)">Del</button><button class="btn-k w" onclick="sk(73)">Ins</button><button class="btn-k w" onclick="sk(74)">Home</button><button class="btn-k w" onclick="sk(77)">End</button><button class="btn-k w" onclick="sk(75)">PgUp</button><button class="btn-k w" onclick="sk(78)">PgDn</button>
</div></div></div>
<div class="acc open" id="a3"><div class="acc-h" onclick="tacc('a3')">Arrow Keys</div><div class="acc-c"><div class="row" style="justify-content:center">
<button class="btn-k" onclick="sk(82)">↑</button></div><div class="row" style="justify-content:center"><button class="btn-k" onclick="sk(80)">←</button><button class="btn-k" onclick="sk(81)">↓</button><button class="btn-k" onclick="sk(79)">→</button>
</div></div></div>
<div class="acc open" id="a4"><div class="acc-h" onclick="tacc('a4')">Shortcuts</div><div class="acc-c"><div class="row">
<button class="btn-s" onclick="ss('c')">Ctrl+C</button><button class="btn-s" onclick="ss('v')">Ctrl+V</button><button class="btn-s" onclick="ss('x')">Ctrl+X</button><button class="btn-s" onclick="ss('z')">Ctrl+Z</button><button class="btn-s" onclick="ss('a')">Ctrl+A</button><button class="btn-s" onclick="ss('s')">Ctrl+S</button>
</div></div></div>
</div>
<div class="panel mouse-panel">
<h2>🖱️ Mouse</h2>
<div class="mp-wrap"><div class="mp" id="pad">Drag to move</div></div>
<div class="mb">
<button id="bl" onmousedown="mp('l')" onmouseup="mr('l')" ontouchstart="mp('l')" ontouchend="mr('l')">Left</button>
<button id="bm" onmousedown="mp('m')" onmouseup="mr('m')" ontouchstart="mp('m')" ontouchend="mr('m')">Middle</button>
<button id="br" onmousedown="mp('r')" onmouseup="mr('r')" ontouchstart="mp('r')" ontouchend="mr('r')">Right</button>
</div>
<div class="row" style="margin-top:8px;align-items:center">
<button class="btn-s" onclick="mc('l')">Click</button>
<button class="btn-s" onclick="mdc('l')">Double</button>
<button class="btn-s" onclick="mc('r')">RClick</button>
<button class="btn-s" onclick="msc(1)">▲</button>
<button class="btn-s" onclick="msc(-1)">▼</button>
<label style="margin:0 0 0 8px">Sens:</label>
<input type="range" id="sens" min="1" max="20" value="8" style="width:80px">
</div>
</div>
</div>
<div class="bottom">
<div class="panel log-panel">
<h2>📋 Log <button class="btn-s" onclick="document.getElementById('log').innerHTML=''" style="float:right;padding:2px 5px;font-size:9px">Clear</button></h2>
<div id="log"></div>
</div>
<div class="panel raw-panel">
<h2>🔧 Raw Command</h2>
<div class="row"><input type="text" id="rc" placeholder="MM:x,y | KT:text | KP:code" style="flex:1"><button class="btn-p" onclick="snd(document.getElementById('rc').value)">Send</button></div>
</div>
</div>
<script>
let ws,dr=!1,lx=0,ly=0,lst=0;
const $=i=>document.getElementById(i);
function log(m,t='info'){const l=$('log'),e=document.createElement('div');e.className=t;e.textContent=`[${new Date().toLocaleTimeString()}] ${m}`;l.appendChild(e);l.scrollTop=l.scrollHeight}
function sts(s){const e=$('st');e.textContent=s==='c'?'Connected':s==='g'?'Connecting...':'Disconnected';e.className='status '+s}
function conn(){log('Connecting...');sts('g');try{ws=new WebSocket(`wss://${location.host}/`);ws.onopen=()=>{log('Connected!','rx');sts('c')};ws.onclose=e=>{log(`Disconnected (${e.code})`);sts('d');ws=null;setTimeout(conn,3000)};ws.onerror=()=>{log('Error','err');sts('d')};ws.onmessage=e=>log(`← ${e.data}`,'rx')}catch(e){log(e.message,'err');sts('d');setTimeout(conn,3000)}}
function snd(c){if(!ws||ws.readyState!==1)return log('Not connected!','err'),!1;log(`→ ${c}`,'tx');ws.send(c);return!0}
function sk(k){snd(`KP:${k}`);setTimeout(()=>snd('KRA'),50)}
function ss(k){snd(`KT:${k}`)}
function hti(e){if(e.key==='Enter'){const i=$('ti');i.value&&snd(`KT:${i.value}`);i.value='';e.preventDefault()}}
function tacc(i){$(i).classList.toggle('open')}
function mc(b){snd(`MC:${b}`)}
function mdc(b){snd(`MC:${b}`);setTimeout(()=>snd(`MC:${b}`),80)}
function mp(b){$('b'+b).classList.add('a');snd(`MP:${b}`)}
function mr(b){$('b'+b).classList.remove('a');snd(`MR:${b}`)}
function msc(d){snd(`MS:${d*3}`)}
function gp(e){return e.touches?{x:e.touches[0].clientX,y:e.touches[0].clientY}:{x:e.clientX,y:e.clientY}}
function ims(e){dr=!0;const p=gp(e);lx=p.x;ly=p.y;e.preventDefault()}
function imm(e){if(!dr)return;const n=Date.now();if(n-lst<50)return;lst=n;const p=gp(e),s=parseInt($('sens').value);let dx=Math.round((p.x-lx)*s/5),dy=Math.round((p.y-ly)*s/5);dx=Math.max(-127,Math.min(127,dx));dy=Math.max(-127,Math.min(127,dy));if(dx||dy){snd(`MM:${dx},${dy}`);lx=p.x;ly=p.y}e.preventDefault()}
function ime(){dr=!1}
document.addEventListener('DOMContentLoaded',()=>{const p=$('pad');p.addEventListener('mousedown',ims);p.addEventListener('mousemove',imm);p.addEventListener('mouseup',ime);p.addEventListener('mouseleave',ime);p.addEventListener('touchstart',ims);p.addEventListener('touchmove',imm);p.addEventListener('touchend',ime);conn()});
</script>
</body>
</html>)rawliteral";


// ============================================================================
// Debug Configuration
// ============================================================================
#define DEBUG_ENABLED 0

#if DEBUG_ENABLED
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(...)
#endif

#endif
