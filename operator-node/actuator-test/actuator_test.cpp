// ============================================================================
// actuator_test.cpp — bench test for both linear actuators + mixer (standalone)
//
// Build/flash with:  pio run -e actuator-test -t upload
// Not part of the main firmware. Covers build-order steps 3 and 4 of the
// 70% handoff: each relay pair drives its actuator both ways, never both
// relays of a pair at once, 100 ms dead time before any reversal.
//
// Wiring (8-ch active-LOW relay board, blue jumper removed):
//   IN1 GPIO25  15 cm EXTEND  (close trapdoor)
//   IN2 GPIO26  15 cm RETRACT (open trapdoor)
//   IN3 GPIO27  40 cm EXTEND  (press)
//   IN4 GPIO33  40 cm RETRACT (home)   <- moved off GPIO14, it pulses at boot
//   IN5 GPIO23  MIXER motor ON (relay in the PSU V+ line to the motor driver)
//   IN6 GPIO32  held OFF (solenoid)
//   Nextion TX -> GPIO16, Nextion RX -> GPIO17, 9600 baud.
//
// Control: tap a button on the Nextion to run that move, tap it again or
// STOP to stop. AUTO RUN plays AUTO[] below, one relay at a time; any tap
// stops it. Serial Monitor (115200) works too: 1..5 = move, a = auto,
// any other key = stop. Phone/laptop: join Wi-Fi "Actuator-Test" (password
// "OperatorPassword"), open http://192.168.4.1. Every move auto-stops after
// RUN_TIMEOUT_MS.
// ============================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#define RXD2 16
#define TXD2 17

// 40 cm at 7 mm/s is ~57 s end to end; built-in end limits stop it anyway
constexpr uint32_t RUN_TIMEOUT_MS = 70000UL;
constexpr uint32_t DEAD_TIME_MS   = 100;

// Relays 1..6 (IN1..IN6). Only 0..4 ever turn on here.
constexpr int RELAY[] = {25, 26, 27, 33, 23, 32};

struct Move { const char *label; int relay; uint16_t x, y, w; };
constexpr uint16_t BH = 95;
const Move MOVES[] = {
  {"15cm CLOSE (R1)", 0,  20,  70, 370},
  {"15cm OPEN (R2)",  1, 410,  70, 370},
  {"40cm PRESS (R3)", 2,  20, 175, 370},
  {"40cm HOME (R4)",  3, 410, 175, 370},
  {"MIXER ON (R5)",   4,  20, 280, 370},
};
constexpr int N_MOVES = sizeof(MOVES) / sizeof(MOVES[0]);
constexpr uint16_t AUTO_X = 410, AUTO_Y = 280, AUTO_W = 370;

// Automated run: MOVES index + how long it stays on. Tune times on the rig.
// Strokes are 7 mm/s no-load plus margin; end limits stop the actuator early.
struct AutoStep { int move; uint32_t ms; };
const AutoStep AUTO[] = {
  {4,  5000},   // mix
  {1, 25000},   // open trapdoor (15 cm = ~21 s)
  {4,  2000},   // mixer burst pushes the mix through
  {0, 25000},   // close trapdoor
  {2, 65000},   // press down (40 cm = ~57 s)
  {3, 65000},   // press home
};
constexpr int N_AUTO = sizeof(AUTO) / sizeof(AUTO[0]);
constexpr uint16_t STOP_X = 20, STOP_Y = 385, STOP_W = 760, STOP_H = 85;

int      active   = -1;  // index into MOVES, -1 = all stopped
int      autoStep = -1;  // index into AUTO, -1 = not in an auto run
uint32_t startMs  = 0;

String runText() {
  String s = autoStep >= 0 ? "AUTO " + String(autoStep + 1) + "/" + String(N_AUTO) + " "
                           : String("RUNNING ");
  return s + MOVES[active].label;
}

// ── Relays ───────────────────────────────────────────────────────────────────

void allOff() {
  for (int pin : RELAY) digitalWrite(pin, HIGH);  // active-LOW: HIGH = off
}

// ── Nextion ──────────────────────────────────────────────────────────────────

void cmd(const String &c) {
  Serial2.print(c);
  Serial2.write(0xFF); Serial2.write(0xFF); Serial2.write(0xFF);
}

void button(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
            const char *color, const char *text) {
  cmd("xstr " + String(x) + "," + String(y) + "," + String(w) + "," + String(h) +
      ",0,WHITE," + color + ",1,1,1,\"" + text + "\"");
}

void draw(const char *status) {
  cmd("fill 0,0,800,60,BLACK");
  cmd(String("xstr 20,10,760,40,0,YELLOW,BLACK,0,1,1,\"") + status + "\"");
  for (int i = 0; i < N_MOVES; i++) {
    const Move &m = MOVES[i];
    button(m.x, m.y, m.w, BH, i == active ? "GREEN" : "GRAY", m.label);
  }
  button(AUTO_X, AUTO_Y, AUTO_W, BH, autoStep >= 0 ? "GREEN" : "BLUE", "AUTO RUN");
  button(STOP_X, STOP_Y, STOP_W, STOP_H, "RED", "STOP ALL");
}

void initScreen() {
  cmd("");             // flush any partial command in the display's parser
  cmd("page 0");
  cmd("tm0.en=0");     // factory demo: stop its page slideshow
  cmd("tsw 255,0");    // ignore the page's own components, we hit-test
  cmd("sendxy=1");     // 0x67 touch-coordinate frames
  cmd("cls BLACK");
  draw("READY - all relays OFF");
}

// ── Control ──────────────────────────────────────────────────────────────────

void stop(const char *why) {
  allOff();
  active   = -1;
  autoStep = -1;
  Serial.printf("STOP (%s)\n", why);
  draw(why);
}

void run(int i) {
  allOff();
  delay(DEAD_TIME_MS);  // let the previous contact open before reversing
  digitalWrite(RELAY[MOVES[i].relay], LOW);
  active  = i;
  startMs = millis();
  Serial.printf("RUN %s (GPIO%d LOW)\n", runText().c_str(), RELAY[MOVES[i].relay]);
  draw(runText().c_str());
}

void start(int i) {
  // Same button again, or any move during an auto run = stop
  if (i == active || autoStep >= 0) { stop("stopped"); return; }
  run(i);
}

void startAuto() {
  if (autoStep >= 0) { stop("stopped"); return; }
  autoStep = 0;
  run(AUTO[0].move);
}

void autoTick() {
  if (autoStep < 0 || millis() - startMs < AUTO[autoStep].ms) return;
  if (++autoStep == N_AUTO) stop("AUTO done - all relays OFF");
  else run(AUTO[autoStep].move);
}

void onTouch(uint16_t x, uint16_t y) {
  for (int i = 0; i < N_MOVES; i++) {
    const Move &m = MOVES[i];
    if (x >= m.x && x < m.x + m.w && y >= m.y && y < m.y + BH) { start(i); return; }
  }
  if (x >= AUTO_X && x < AUTO_X + AUTO_W && y >= AUTO_Y && y < AUTO_Y + BH) { startAuto(); return; }
  // Anywhere else, STOP bar included, stops everything
  if (active >= 0) stop("stopped");
}

// ── Nextion RX framing ───────────────────────────────────────────────────────

uint8_t buf[16];
int len = 0, ffCount = 0;

void handleFrame() {
  if (len == 6 && buf[0] == 0x67 && buf[5] == 1) {   // touch press
    onTouch((buf[1] << 8) | buf[2], (buf[3] << 8) | buf[4]);
  } else if (len >= 1 && buf[0] == 0x88) {           // display rebooted
    Serial.println("Nextion restarted, redrawing");
    initScreen();
  }
}

void pumpRx() {
  while (Serial2.available()) {
    uint8_t b = Serial2.read();
    if (b == 0xFF) {
      if (++ffCount == 3) { handleFrame(); len = 0; ffCount = 0; }
      continue;
    }
    while (ffCount > 0 && len < (int)sizeof(buf)) { buf[len++] = 0xFF; ffCount--; }
    ffCount = 0;
    if (len < (int)sizeof(buf)) buf[len++] = b;
  }
}

// ── Web control ──────────────────────────────────────────────────────────────

WebServer web(80);

const char PAGE[] PROGMEM = R"HTML(<!doctype html><meta name=viewport content="width=device-width,initial-scale=1">
<title>Actuator Test</title>
<style>body{font:18px sans-serif;background:#111;color:#eee;margin:16px}
button{width:100%;padding:24px;margin:6px 0;font-size:20px;border:0;border-radius:8px;background:#555;color:#fff}
.on{background:#16a34a}#s{background:#dc2626}p{color:#facc15}</style>
<p id=st>...</p>
<button data-i=0>15cm CLOSE (R1)</button><button data-i=1>15cm OPEN (R2)</button>
<button data-i=2>40cm PRESS (R3)</button><button data-i=3>40cm HOME (R4)</button>
<button data-i=4>MIXER ON (R5)</button><button id=a>AUTO RUN</button>
<button id=s>STOP ALL</button>
<script>
const b=[...document.querySelectorAll('[data-i]')];
function show(t){let [a,m]=t.split('|');a=+a;st.textContent=m;
b.forEach(x=>x.className=+x.dataset.i===a?'on':'');
document.getElementById('a').className=m.startsWith('AUTO')?'on':''}
const go=u=>fetch(u).then(r=>r.text()).then(show).catch(()=>st.textContent='NO LINK');
b.forEach(x=>x.onclick=()=>go('/move?i='+x.dataset.i));
s.onclick=()=>go('/stop');
document.getElementById('a').onclick=()=>go('/auto');
setInterval(()=>go('/state'),1000);go('/state');
</script>)HTML";

// "index|status": active move index (-1 = stopped), then a status line
void sendState() {
  String s = String(active) + "|";
  if (active < 0) s += "STOPPED - all relays OFF";
  else s += runText() + "  " + String((millis() - startMs) / 1000) + " s";
  web.send(200, "text/plain", s);
}

void webBegin() {
  WiFi.softAP("Actuator-Test", "OperatorPassword");
  web.on("/", [] { web.send_P(200, "text/html", PAGE); });
  web.on("/state", sendState);
  web.on("/stop", [] { if (active >= 0) stop("stopped (web)"); sendState(); });
  web.on("/auto", [] { startAuto(); sendState(); });
  web.on("/move", [] {
    int i = web.arg("i").toInt();
    if (web.hasArg("i") && i >= 0 && i < N_MOVES) start(i);
    sendState();
  });
  web.begin();
  Serial.printf("Web: join Wi-Fi Actuator-Test, open http://%s\n",
                WiFi.softAPIP().toString().c_str());
}

// ── Arduino ──────────────────────────────────────────────────────────────────

void setup() {
  // Off level BEFORE pinMode so no relay clicks during boot
  for (int pin : RELAY) { digitalWrite(pin, HIGH); pinMode(pin, OUTPUT); }

  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  delay(500);
  Serial.println("\n===== Actuator test: 1..5 = move, a = auto, any other key = stop =====");
  initScreen();
  webBegin();
}

void loop() {
  pumpRx();
  web.handleClient();

  if (Serial.available()) {
    char c = Serial.read();
    if (c >= '1' && c < '1' + N_MOVES) start(c - '1');
    else if (c == 'a') startAuto();
    else if (c != '\n' && c != '\r' && active >= 0) stop("stopped");
  }

  autoTick();
  if (active >= 0 && millis() - startMs >= RUN_TIMEOUT_MS) stop("TIMEOUT - auto stop");
}
