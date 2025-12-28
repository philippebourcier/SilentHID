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
<title>SilentHID</title>
<style>
*{box-sizing:border-box;font-family:system-ui,sans-serif;margin:0;padding:0}
body{background:#0d1117;color:#e6edf3;padding:12px;height:100vh;display:flex;flex-direction:column;overflow:hidden}
.hdr{display:flex;justify-content:space-between;align-items:center;padding-bottom:10px;border-bottom:1px solid #30363d;margin-bottom:10px;flex-shrink:0}
.hdr h1{color:#58a6ff;font-size:1.3em;display:flex;align-items:center;gap:8px}
.hdr .sub{color:#7d8590;font-size:.75em;margin-left:8px}
.sts{padding:4px 12px;border-radius:12px;font-size:.7em;font-weight:500}
.sts.c{background:#238636;color:#fff}.sts.d{background:#da3633;color:#fff}.sts.g{background:#9e6a03;color:#fff}
.main{display:flex;gap:12px;flex:1;min-height:0}
.col{display:flex;flex-direction:column;gap:10px}
.col-kb{width:510px;flex-shrink:0}
.col-mouse{flex:1;display:flex;gap:12px}
.pnl{background:#161b22;border:1px solid #30363d;border-radius:6px;padding:10px}
.pnl h2{font-size:.8em;color:#58a6ff;margin-bottom:8px;display:flex;align-items:center;gap:6px}
.pnl h3{font-size:.7em;color:#7d8590;margin:8px 0 6px}
.row{display:flex;gap:6px;flex-wrap:wrap;align-items:center}
.row+.row{margin-top:6px}
input,select{padding:5px 8px;border:1px solid #30363d;border-radius:4px;background:#0d1117;color:#e6edf3;font-size:11px}
input:focus{outline:none;border-color:#58a6ff}
input[type=number]{width:60px}
button{padding:5px 10px;border:none;border-radius:4px;cursor:pointer;font-size:11px;font-weight:500}
.bp{background:#238636;color:#fff}.bs{background:#30363d;color:#e6edf3}.bd{background:#1f6feb;color:#fff}.bb{background:#1f6feb;color:#fff}
.bk{min-width:32px;height:32px;background:#21262d;color:#e6edf3;border:1px solid #30363d;font-size:10px;border-radius:4px}
.bk:hover{background:#30363d}.bk.w{min-width:52px}
.ki{width:100%;padding:8px;font-size:12px;background:#0d1117;border:1px solid #30363d;border-radius:4px;color:#e6edf3;margin-bottom:8px}
.ks{background:#0d1117;border-radius:4px;padding:8px;margin-bottom:6px}
.ks-t{font-size:.65em;color:#7d8590;margin-bottom:6px}
.mpad{background:#0d1117;border:2px solid #30363d;border-radius:6px;flex:1;cursor:crosshair;touch-action:none;position:relative;display:flex;align-items:center;justify-content:center;min-height:120px}
.mpad:hover{border-color:#58a6ff}
.mpad .lbl{color:#484f58;font-size:11px;pointer-events:none}
.mpad .crd{position:absolute;bottom:4px;right:6px;font-size:9px;color:#484f58;font-family:monospace}
.mpad .cross{position:absolute;width:10px;height:10px;border:2px solid #58a6ff;border-radius:50%;pointer-events:none;transform:translate(-50%,-50%)}
.mpad .grid{position:absolute;inset:0;pointer-events:none;opacity:.3;height:100%;width:100%}
.mbtn{display:grid;grid-template-columns:repeat(3,1fr);gap:6px;margin-top:8px}
.mbtn button{height:36px;background:#21262d;border:1px solid #30363d;color:#e6edf3;font-size:11px}
.mbtn button.a{background:#1f6feb;border-color:#1f6feb}
.actl{background:#1c1e24;border:1px solid #30363d;border-radius:4px;padding:8px;margin-bottom:8px}
.actl h4{font-size:.65em;color:#d29922;margin-bottom:6px}
.qb{display:flex;flex-wrap:wrap;gap:3px}
.qb button{font-size:9px;padding:4px 6px;background:#21262d;border:1px solid #30363d;color:#7d8590}
.btm{display:flex;gap:12px;flex-shrink:0;margin-top:10px}
.btm .pnl{flex:1}
.log{background:#0d1117;border:1px solid #30363d;border-radius:4px;padding:6px;height:172px;overflow-y:auto;font-family:monospace;font-size:10px}
.log .tx{color:#58a6ff}.log .rx{color:#3fb950}.log .err{color:#f85149}
.mpnl{flex:1;display:flex;flex-direction:column}
</style>
</head>
<body>
<div class="hdr"><h1>🎮 SilentHID Control<span class="sub">USB HID over WebSocket</span></h1><span id="st" class="sts g">...</span></div>
<div class="main">
<div class="col col-kb">
<div class="pnl" style="flex:1;display:flex;flex-direction:column">
<h2>⌨️ Keyboard</h2>
<input type="text" class="ki" id="ti" placeholder="Type + Enter..." onkeydown="hti(event)">
<div class="ks"><div class="ks-t">Function Keys</div><div class="row">
<button class="bk" onclick="sk(58)">F1</button><button class="bk" onclick="sk(59)">F2</button><button class="bk" onclick="sk(60)">F3</button><button class="bk" onclick="sk(61)">F4</button><button class="bk" onclick="sk(62)">F5</button><button class="bk" onclick="sk(63)">F6</button><button class="bk" onclick="sk(64)">F7</button><button class="bk" onclick="sk(65)">F8</button><button class="bk" onclick="sk(66)">F9</button><button class="bk" onclick="sk(67)">F10</button><button class="bk" onclick="sk(68)">F11</button><button class="bk" onclick="sk(69)">F12</button>
</div></div>
<div class="ks"><div class="ks-t">Special</div><div class="row">
<button class="bk w" onclick="sk(41)">Esc</button><button class="bk w" onclick="sk(43)">Tab</button><button class="bk w" onclick="sk(40)">Ent</button><button class="bk w" onclick="sk(42)">Bksp</button><button class="bk w" onclick="sk(76)">Del</button><button class="bk w" onclick="sk(73)">Ins</button><button class="bk w" onclick="sk(74)">Hom</button><button class="bk w" onclick="sk(77)">End</button>
</div></div>
<div class="ks"><div class="ks-t">Nav & Shortcuts</div>
<div class="row" style="justify-content:center"><button class="bk" onclick="sk(82)">↑</button></div>
<div class="row" style="justify-content:center"><button class="bk" onclick="sk(80)">←</button><button class="bk" onclick="sk(81)">↓</button><button class="bk" onclick="sk(79)">→</button></div>
<div class="row" style="margin-top:36px"><button class="bs" onclick="ss('c')">^C</button><button class="bs" onclick="ss('v')">^V</button><button class="bs" onclick="ss('x')">^X</button><button class="bs" onclick="ss('z')">^Z</button><button class="bs" onclick="ss('a')">^A</button><button class="bs" onclick="ss('s')">^S</button></div>
</div>
<div style="flex:1"></div>
<div class="ks" style="margin:0"><div class="ks-t">Raw Command</div>
<div class="row"><input type="text" id="rc" placeholder="MA:x,y | MF:r,r | KT:txt" style="flex:1" onkeydown="if(event.key==='Enter')snd(this.value)"><button class="bb" onclick="snd($('rc').value)">Send</button></div>
</div>
</div>
</div>
<div class="col-mouse">
<div class="pnl mpnl">
<h2>↔️ Relative Mouse</h2>
<div class="mpad" id="padRel"><span class="lbl">Drag to move</span><div class="crd" id="crdRel"></div></div>
<div class="mbtn">
<button id="bl" onmousedown="mp('l')" onmouseup="mr('l')">Left</button>
<button id="bm" onmousedown="mp('m')" onmouseup="mr('m')">Mid</button>
<button id="br" onmousedown="mp('r')" onmouseup="mr('r')">Right</button>
</div>
<div class="row" style="margin-top:6px">
<button class="bs" onclick="mc('l')">Click</button><button class="bs" onclick="mdc('l')">Dbl</button><button class="bs" onclick="msc(3)">▲</button><button class="bs" onclick="msc(-3)">▼</button>
<label style="font-size:10px;color:#7d8590">Spd:</label><input type="range" id="sens" min="1" max="20" value="8" style="width:60px">
</div>
</div>
<div class="pnl mpnl">
<h2>📍 Absolute Mouse</h2>
<div class="actl">
<div class="row"><label style="font-size:10px;color:#7d8590">Scr:</label>
<input type="number" id="scrW" value="1920"><span style="color:#484f58">×</span><input type="number" id="scrH" value="1080">
<button class="bd" onclick="setRes()">Set</button><button class="bs" onclick="getRes()">Get</button><span id="resD" style="font-size:9px;color:#7d8590"></span></div>
<div class="row"><label style="font-size:10px;color:#7d8590">Pos:</label>
<input type="number" id="absX" value="960"><span style="color:#484f58">,</span><input type="number" id="absY" value="540">
<button class="bd" onclick="mvA()">Go</button><button class="bp" onclick="clkA('l')">Clk</button></div>
<div class="row"><label style="font-size:10px;color:#7d8590">Rat:</label>
<input type="number" id="ratX" value="0.5" step="0.001" style="width:55px"><span style="color:#484f58">,</span><input type="number" id="ratY" value="0.5" step="0.001" style="width:55px">
<button class="bd" onclick="mvR()">Go</button><button class="bp" onclick="clkR('l')">Clk</button></div>
<div class="row"><label style="font-size:10px;color:#7d8590">Qk:</label><div class="qb">
<button onclick="qp(0,0)">↖ TL</button>
<button onclick="qp(0.5,0)">↑ T</button>
<button onclick="qp(1,0)">↗ TR</button>
<button onclick="qp(0,0.5)">← L</button>
<button onclick="qp(0.5,0.5)">● C</button>
<button onclick="qp(1,0.5)">→ R</button>
<button onclick="qp(0,1)">↙ BL</button>
<button onclick="qp(0.5,1)">↓ B</button>
<button onclick="qp(1,1)">↘ BR</button>
</div></div>
</div>
<div class="mpad abs" id="padAbs"><svg class="grid" id="gSvg"></svg><div class="cross" id="cross" style="display:none"></div><span class="lbl">Click to position</span><div class="crd" id="crdAbs"></div></div>
</div>
</div>
</div>
<div class="btm">
<div class="pnl"><h2>📋 Log <button class="bs" onclick="$('log').innerHTML=''" style="float:right;padding:2px 6px;font-size:9px">Clr</button></h2><div class="log" id="log"></div></div>
</div>
<script>
var ws,dr=0,lx=0,ly=0,lt=0,sW=1920,sH=1080,$=function(i){return document.getElementById(i)};
function log(m,t){var l=$('log'),e=document.createElement('div');e.className=t||'';e.textContent='['+new Date().toLocaleTimeString()+'] '+m;l.appendChild(e);l.scrollTop=l.scrollHeight}
function sts(s){var e=$('st');e.textContent=s=='c'?'Connected':s=='g'?'Connecting...':'Disconnected';e.className='sts '+s}
function conn(){log('Connecting...');sts('g');try{ws=new WebSocket('wss://'+location.host+'/');ws.onopen=function(){log('Connected','rx');sts('c');getRes()};ws.onclose=function(){log('Disconnected');sts('d');ws=null;setTimeout(conn,3e3)};ws.onerror=function(){log('Error','err');sts('d')};ws.onmessage=function(e){log('← '+e.data,'rx');pRes(e.data)}}catch(e){log(e.message,'err');sts('d');setTimeout(conn,3e3)}}
function pRes(d){var p;if(d.indexOf('OK:')==0){p=d.substring(3).split(',');if(p.length==2){sW=+p[0];sH=+p[1];uRes()}}else if(/^\d+,\d+$/.test(d)){p=d.split(',');sW=+p[0];sH=+p[1];uRes()}}
function uRes(){$('resD').textContent='('+sW+'×'+sH+')';$('scrW').value=sW;$('scrH').value=sH;dG()}
function snd(c){if(!ws||ws.readyState!=1)return log('Not connected','err'),0;log('→ '+c,'tx');ws.send(c);return 1}
function sk(k){snd('KP:'+k);setTimeout(function(){snd('KRA')},50)}
function ss(k){snd('KT:'+k)}
function hti(e){if(e.key=='Enter'){var i=$('ti');if(i.value){snd('KT:'+i.value);i.value=''}e.preventDefault()}}
function mc(b){snd('MC:'+b)}
function mdc(b){snd('MC:'+b);setTimeout(function(){snd('MC:'+b)},100)}
function mp(b){$('b'+b).classList.add('a');snd('MP:'+b)}
function mr(b){$('b'+b).classList.remove('a');snd('MR:'+b)}
function msc(d){snd('MS:'+d)}
function setRes(){sW=+$('scrW').value||1920;sH=+$('scrH').value||1080;snd('SR:'+sW+','+sH)}
function getRes(){snd('GR')}
function mvA(){snd('MA:'+(+$('absX').value||0)+','+(+$('absY').value||0))}
function clkA(b){snd('CA:'+(+$('absX').value||0)+','+(+$('absY').value||0)+','+b)}
function mvR(){snd('MF:'+(+$('ratX').value||0)+','+(+$('ratY').value||0))}
function clkR(b){snd('CF:'+(+$('ratX').value||0)+','+(+$('ratY').value||0)+','+b)}
function qp(rx,ry){$('ratX').value=rx;$('ratY').value=ry;$('absX').value=Math.round(rx*sW);$('absY').value=Math.round(ry*sH);snd('MF:'+rx+','+ry)}
function dG(){var s=$('gSvg'),p=$('padAbs'),w=p.clientWidth,h=p.clientHeight,g='',i;for(i=1;i<4;i++)g+='<line x1="'+w*i/4+'" y1="0" x2="'+w*i/4+'" y2="'+h+'" stroke="#30363d"/><line x1="0" y1="'+h*i/4+'" x2="'+w+'" y2="'+h*i/4+'" stroke="#30363d"/>';g+='<line x1="'+w/2+'" y1="0" x2="'+w/2+'" y2="'+h+'" stroke="#9e6a03" stroke-dasharray="4"/><line x1="0" y1="'+h/2+'" x2="'+w+'" y2="'+h/2+'" stroke="#9e6a03" stroke-dasharray="4"/>';s.innerHTML=g}
function gp(e){return e.touches?{x:e.touches[0].clientX,y:e.touches[0].clientY}:{x:e.clientX,y:e.clientY}}
function gpp(pad,e){var r=pad.getBoundingClientRect(),pt=gp(e);return{x:Math.max(0,Math.min(1,(pt.x-r.left)/r.width)),y:Math.max(0,Math.min(1,(pt.y-r.top)/r.height)),px:pt.x-r.left,py:pt.y-r.top}}
function rS(e){dr=1;var p=gp(e);lx=p.x;ly=p.y;e.preventDefault()}
function rM(e){var pos=gpp($('padRel'),e),p,s,dx,dy,n;$('crdRel').textContent='Δ '+Math.round(gp(e).x-lx)+','+Math.round(gp(e).y-ly);if(!dr)return;n=Date.now();if(n-lt<50)return;lt=n;p=gp(e);s=+$('sens').value;dx=Math.round((p.x-lx)*s/5);dy=Math.round((p.y-ly)*s/5);dx=Math.max(-127,Math.min(127,dx));dy=Math.max(-127,Math.min(127,dy));if(dx||dy){snd('MM:'+dx+','+dy);lx=p.x;ly=p.y}e.preventDefault()}
function rE(){dr=0}
function aM(e){var pos=gpp($('padAbs'),e),c=$('cross');c.style.display='block';c.style.left=pos.px+'px';c.style.top=pos.py+'px';$('crdAbs').textContent=Math.round(pos.x*sW)+','+Math.round(pos.y*sH)+' ('+pos.x.toFixed(3)+','+pos.y.toFixed(3)+')';$('ratX').value=pos.x.toFixed(3);$('ratY').value=pos.y.toFixed(3);$('absX').value=Math.round(pos.x*sW);$('absY').value=Math.round(pos.y*sH);if(e.buttons==1){var n=Date.now();if(n-lt<30)return;lt=n;snd('MF:'+pos.x.toFixed(4)+','+pos.y.toFixed(4))}}
function aC(e){var pos=gpp($('padAbs'),e);snd('CF:'+pos.x.toFixed(4)+','+pos.y.toFixed(4)+',l')}
document.addEventListener('DOMContentLoaded',function(){var pR=$('padRel'),pA=$('padAbs');pR.onmousedown=rS;pR.onmousemove=rM;pR.onmouseup=rE;pR.onmouseleave=rE;pR.ontouchstart=rS;pR.ontouchmove=rM;pR.ontouchend=rE;pA.onmousemove=aM;pA.onclick=aC;pA.ontouchmove=function(e){aM(e);e.preventDefault()};pA.ontouchend=function(e){var t=e.changedTouches[0];aC({clientX:t.clientX,clientY:t.clientY})};window.onresize=dG;conn();setTimeout(dG,100)});
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
