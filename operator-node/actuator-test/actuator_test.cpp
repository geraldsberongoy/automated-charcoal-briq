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
//   IN6 GPIO32  WATER solenoid valve open
//   YF-S201 flow sensor signal -> GPIO34 through a 10k series resistor, no
//   pull-up (with one, the two halve the pulse and nothing counts)
//   HX711 DT -> GPIO19, SCK -> GPIO18, VCC -> 3V3 (S-type load cell on E+/E-/A+/A-)
//   Nextion TX -> GPIO16, Nextion RX -> GPIO17, 115200 baud (boots at 9600).
//
// Control: tap a button on the Nextion to run that move, tap it again or
// STOP to stop. AUTO RUN plays AUTO[] below, one relay at a time; any tap
// stops it. Serial Monitor (115200) works too: 1..6 = move, a = auto, t = tare,
// any other key = stop. Phone/laptop (only with WEB_ENABLED): join Wi-Fi
// "Actuator-Test" (password "OperatorPassword"), open http://192.168.4.1.
// Every move auto-stops after RUN_TIMEOUT_MS.
// ============================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#define RXD2 16
#define TXD2 17

// 40 cm at 7 mm/s is ~57 s end to end; built-in end limits stop it anyway
constexpr uint32_t RUN_TIMEOUT_MS = 70000UL;
constexpr uint32_t DEAD_TIME_MS   = 100;

// Phone web page. Needs ESP32 VIN and the Nextion on the 5 V power module:
// on laptop USB alone the Wi-Fi start-up current kills the ESP32 ~2 s in.
constexpr bool WEB_ENABLED = true;

// Relays 1..6 (IN1..IN6)
constexpr int RELAY[] = {25, 26, 27, 33, 23, 32};
constexpr int WATER_RELAY = 5;

// YF-S201: calibration from the README (gravimetric test). Re-check it by
// filling a measuring jug: new = 450 * litres shown / litres in the jug.
constexpr int   FLOW_PIN       = 34;     // input-only, interrupt, no boot role
constexpr float PULSES_PER_L   = 450.0f;
constexpr float WATER_TARGET_L = 1.0f;   // AUTO RUN water dose

// Flow sensor off for now: GPIO34 kept counting with no flow. While off,
// nothing reads the pin and AUTO RUN's water step runs on time instead.
constexpr bool     FLOW_ENABLED       = false;
constexpr uint32_t WATER_NO_SENSOR_MS = 10000;

// HX711 + S-type load cell. Scale from the README (22,480 counts/kg), which
// was taken at 5 V excitation; here the HX711 runs on 3V3, so re-check it
// with a known weight: new = (raw - tare) / kg on the scale.
constexpr int   HX_DT       = 19;
constexpr int   HX_SCK      = 18;
constexpr float LSB_PER_KG  = 22480.0f;
constexpr float PRESS_STOP_KG = 10.0f;   // press stops pushing at this load

long hxRaw  = 0;      // last reading
long hxTare = 0;      // raw value at zero load
bool hxSeen = false;  // has the HX711 ever answered

// DT goes LOW when a sample is ready. Then 24 clocks shift it out MSB
// first; a 25th selects channel A, gain 128 for the next one.
bool hxReady() { return digitalRead(HX_DT) == LOW; }

long hxRead() {
  static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
  long v = 0;
  portENTER_CRITICAL(&mux);   // ~60 us; a long SCK-high stall resets the HX711
  for (int i = 0; i < 25; i++) {
    digitalWrite(HX_SCK, HIGH);
    delayMicroseconds(1);
    if (i < 24) v = (v << 1) | digitalRead(HX_DT);
    digitalWrite(HX_SCK, LOW);
    delayMicroseconds(1);
  }
  portEXIT_CRITICAL(&mux);
  if (v & 0x800000) v |= ~0xFFFFFFL;   // 24-bit two's complement
  return v;
}

struct Move { const char *label; int relay; uint16_t x, y, w; };
constexpr uint16_t BH = 75;
const Move MOVES[] = {
  {"15cm CLOSE (R1)", 0,  20,  65, 370},
  {"15cm OPEN (R2)",  1, 410,  65, 370},
  {"40cm PRESS (R3)", 2,  20, 148, 370},
  {"40cm HOME (R4)",  3, 410, 148, 370},
  {"MIXER ON (R5)",   4,  20, 231, 370},
  {"WATER ON (R6)",   5, 410, 231, 370},
};
constexpr int N_MOVES = sizeof(MOVES) / sizeof(MOVES[0]);
constexpr uint16_t AUTO_X = 20, AUTO_Y = 314, AUTO_W = 760;

// Automated run: MOVES index, max time on, and for water the litres that end
// the step early. Strokes are 7 mm/s no-load; end limits stop them anyway.
struct AutoStep { int move; uint32_t ms; float litres; };
const AutoStep AUTO[] = {
  // water in: to the dose with the sensor (60 s cap), else a fixed time
  {5, FLOW_ENABLED ? 60000 : WATER_NO_SENSOR_MS, FLOW_ENABLED ? WATER_TARGET_L : 0},
  {4,  5000, 0},   // mix
  {1, 20000, 0},   // open trapdoor (15 cm = ~21 s)
  {4,  5000, 0},   // mixer burst pushes the mix through
  {0, 20000, 0},   // close trapdoor
  {2, 20000, 0},   // press down (40 cm = ~57 s, so ~14 cm here)
  {3, 20000, 0},   // press home
};
constexpr int N_AUTO = sizeof(AUTO) / sizeof(AUTO[0]);
constexpr uint16_t STOP_X = 20, STOP_Y = 397, STOP_W = 760, STOP_H = 75;

int      active   = -1;  // index into MOVES, -1 = all stopped
int      autoStep = -1;  // index into AUTO, -1 = not in an auto run
uint32_t startMs  = 0;

volatile uint32_t flowPulses = 0;
uint32_t pulseStart = 0;   // flowPulses when the current move began
uint32_t rawUntilMs = 0;   // serial key r: raw GPIO34 voltage readout until then

// Plain counter: a 2 ms gap filter here zeroed real pulses on the rig.
// ponytail: no noise filter, so a loose signal wire counts junk. Seat it.
void IRAM_ATTR onFlowPulse() { flowPulses++; }

float litres() { return (flowPulses - pulseStart) / PULSES_PER_L; }
bool  watering() { return active >= 0 && MOVES[active].relay == WATER_RELAY; }

String runText() {
  String s = autoStep >= 0 ? "AUTO " + String(autoStep + 1) + "/" + String(N_AUTO) + " "
                           : String("RUNNING ");
  s += MOVES[active].label;
  if (FLOW_ENABLED && watering()) s += "  " + String(litres(), 2) + " L";
  return s;
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

void statusLine(const char *status) {
  cmd("fill 0,0,400,60,BLACK");
  cmd(String("xstr 20,10,380,40,0,YELLOW,BLACK,0,1,1,\"") + status + "\"");
}

// Total flow since power-up, always on screen
void flowLine() {
  String t = FLOW_ENABLED ? "FLOW " + String(flowPulses / PULSES_PER_L, 2) + " L" : String("FLOW off");
  cmd("xstr 400,10,185,40,0,WHITE,BLACK,2,1,1,\"" + t + "\"");
}

float loadKg() { return (hxRaw - hxTare) / LSB_PER_KG; }

// Load cell force, top right, always on screen
void forceLine() {
  String t = hxSeen ? "LOAD " + String(loadKg(), 2) + " kg" : String("LOAD --");
  cmd("xstr 590,10,190,40,0,CYAN,BLACK,2,1,1,\"" + t + "\"");
}

void draw(const char *status) {
  statusLine(status);
  flowLine();
  forceLine();
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
  delay(300);          // page load drops commands sent during it
  cmd("tm0.en=0");     // factory demo: stop its page slideshow
  cmd("tsw 255,0");    // ignore the page's own components, we hit-test
  cmd("sendxy=1");     // 0x67 touch-coordinate frames
  cmd("cls BLACK");
  draw(active >= 0 ? runText().c_str() : "READY - all relays OFF");
}

// ── Control ──────────────────────────────────────────────────────────────────

void stop(const char *why) {
  if (FLOW_ENABLED && watering()) Serial.printf("Water: %.2f L\n", litres());
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
  active     = i;
  startMs    = millis();
  pulseStart = flowPulses;
  Serial.printf("%s (GPIO%d LOW)\n", runText().c_str(), RELAY[MOVES[i].relay]);
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

void nextAutoStep();

void autoTick() {
  if (autoStep < 0) return;
  const AutoStep &st = AUTO[autoStep];
  const bool dosed = st.litres > 0 && litres() >= st.litres;
  if (!dosed && millis() - startMs < st.ms) return;
  if (FLOW_ENABLED && watering()) Serial.printf("Water: %.2f L%s\n", litres(), dosed ? "" : " (time cap hit)");
  nextAutoStep();
}

void nextAutoStep() {
  if (++autoStep == N_AUTO) stop("AUTO done - all relays OFF");
  else run(AUTO[autoStep].move);
}

// Press (40 cm EXTEND, relay index 2) stops once the load cell reaches
// PRESS_STOP_KG. In AUTO RUN the press step ends early and the run goes on
// to HOME; a manual press just stops. HOME is never blocked: it unloads.
void loadCutoff() {
  if (active < 0 || MOVES[active].relay != 2 || loadKg() < PRESS_STOP_KG) return;
  Serial.printf("LOAD %.2f kg >= %.1f kg: press stopped\n", loadKg(), PRESS_STOP_KG);
  if (autoStep >= 0) nextAutoStep();
  else stop("LOAD limit - press stopped");
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
uint32_t lastRxMs = 0, lastPingMs = 0;

void handleFrame() {
  lastRxMs = millis();
  if (len >= 2 && buf[0] == 0x66 && buf[1] != 0) {   // sendme: off page 0
    Serial.println("Nextion left page 0, redrawing");
    initScreen();
  } else if (len == 6 && buf[0] == 0x67 && buf[5] == 1) {   // touch press
    onTouch((buf[1] << 8) | buf[2], (buf[3] << 8) | buf[4]);
  } else if (len >= 1 && buf[0] == 0x88) {           // display rebooted
    Serial.println("Nextion restarted, redrawing");
    initScreen();
  }
}

// 9600 baud made a full redraw block loop() ~600 ms. Same handshake as
// NextionManager::_syncLink(): the display boots at 9600, so ask for 115200
// there, then follow it. Harmless if it is already at 115200.
void syncLink() {
  Serial2.updateBaudRate(9600);
  cmd("");
  cmd("baud=115200");
  Serial2.flush();
  delay(100);
  Serial2.updateBaudRate(115200);
  delay(50);
  while (Serial2.available()) Serial2.read();  // drop replies from the switch
  len = 0;
  ffCount = 0;
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
<button data-i=4>MIXER ON (R5)</button><button data-i=5>WATER ON (R6)</button>
<button id=a>AUTO RUN</button>
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
  s += FLOW_ENABLED ? "   |   FLOW " + String(flowPulses / PULSES_PER_L, 2) + " L (" + String(flowPulses) + " pulses)"
                    : String("   |   FLOW off");
  s += hxSeen ? "   |   LOAD " + String(loadKg(), 2) + " kg (" + String(loadKg() * 9.80665f, 0) + " N)"
              : String("   |   LOAD no HX711");
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
  if (FLOW_ENABLED) {
    pinMode(FLOW_PIN, INPUT);   // no pull-up on GPIO34; the sensor has its own
    attachInterrupt(digitalPinToInterrupt(FLOW_PIN), onFlowPulse, FALLING);
  }

  pinMode(HX_DT, INPUT_PULLUP);   // unplugged HX711 reads "not ready", not junk
  pinMode(HX_SCK, OUTPUT);
  digitalWrite(HX_SCK, LOW);   // SCK high > 60 us powers the HX711 down

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  delay(500);
  Serial.println("\n===== Actuator test: 1..6 = move, a = auto, any other key = stop =====");
  syncLink();
  initScreen();
  if (WEB_ENABLED) webBegin();
}

// Logs any part of loop() that stalls 250 ms+ (a move start takes ~145 ms,
// 100 of it the deliberate dead time): a stall delays STOP taps and timers
void slow(const char *what, uint32_t t0) {
  const uint32_t d = millis() - t0;
  if (d >= 250) Serial.printf("SLOW %s %lu ms\n", what, (unsigned long)d);
}

void loop() {
  const uint32_t loopStart = millis();
  uint32_t t = millis();
  pumpRx();
  slow("nextion rx/touch", t);
  t = millis();
  if (WEB_ENABLED) web.handleClient();
  slow("web", t);

  // Keepalive: the demo slideshow or a display reboot wipes our screen.
  // sendme replies with the page number; silence means redraw.
  if (millis() - lastPingMs >= 2000) {
    lastPingMs = millis();
    if (millis() - lastRxMs >= 6000) {
      Serial.println("No reply from Nextion, resyncing and redrawing");
      syncLink();   // a display reboot drops it back to 9600
      initScreen();
      lastRxMs = millis();
    }
    cmd("sendme");
  }

  if (Serial.available()) {
    char c = Serial.read();
    if (c >= '1' && c < '1' + N_MOVES) start(c - '1');
    else if (c == 'a') startAuto();
    else if (c == 'r') { rawUntilMs = millis() + 20000; Serial.println("RAW flow readout for 20 s"); }
    else if (c == 't') { hxTare = hxRaw; Serial.printf("LOAD tared at raw %ld\n", hxTare); forceLine(); }
    else if (c != '\n' && c != '\r' && active >= 0) stop("stopped");
  }

  // Live litres on the Nextion while the valve is open (status line only)
  static uint32_t lastFlowDrawMs = 0;
  if (watering() && millis() - lastFlowDrawMs >= 500) {
    lastFlowDrawMs = millis();
    statusLine(runText().c_str());
  }

  // Load cell: read when the HX711 has a sample (~10/s), report once a second
  static uint32_t lastLoadReportMs = 0, lastHxMs = 0;
  // Zero at boot (keep it unloaded): first readings after power-up jump, so
  // skip 20 (~2 s) and average the next 10
  static int  bootReads = 0;
  static long tareSum   = 0;
  if (hxReady()) {
    hxRaw = hxRead();
    lastHxMs = millis();
    if (!hxSeen) Serial.println("LOAD: HX711 found, zeroing in ~3 s, keep it unloaded");
    hxSeen = true;
    if (bootReads < 30) {
      if (++bootReads > 20) tareSum += hxRaw;
      if (bootReads == 30) { hxTare = tareSum / 10; Serial.printf("LOAD tared at raw %ld\n", hxTare); }
    } else {
      loadCutoff();   // only once zeroed, so boot jumps can't trip it
    }
  }
  if (millis() - lastLoadReportMs >= 1000) {
    lastLoadReportMs = millis();
    if (hxSeen && millis() - lastHxMs > 2000) { hxSeen = false; Serial.println("LOAD: HX711 stopped answering"); }
    if (hxSeen) Serial.printf("LOAD  raw %8ld  %7.2f kg  %6.0f N\n", hxRaw, loadKg(), loadKg() * 9.80665f);
    else if ((millis() / 1000) % 5 == 0)
      Serial.println("LOAD  no HX711 (check DT GPIO19, SCK GPIO18, VCC 3V3, GND)");
    forceLine();
  }

  // Serial key r: GPIO34 voltage 10x a second for 20 s, to see what the
  // sensor line does. analogRead takes the pin off the digital input, so
  // the interrupt is re-armed when the window ends.
  static uint32_t lastRawMs = 0;
  static bool rawWasOn = false;
  if (millis() < rawUntilMs) {
    rawWasOn = true;
    if (millis() - lastRawMs >= 100) {
      lastRawMs = millis();
      Serial.printf("RAW GPIO34 %4d mV\n", analogReadMilliVolts(FLOW_PIN));
    }
  } else if (rawWasOn) {
    rawWasOn = false;
    if (FLOW_ENABLED) {
      pinMode(FLOW_PIN, INPUT);
      attachInterrupt(digitalPinToInterrupt(FLOW_PIN), onFlowPulse, FALLING);
    }
    Serial.println("RAW done");
  }

  // Flow test: new pulses update the FLOW box and serial once a second
  static uint32_t lastFlowReportMs = 0, lastPulses = 0;
  if (millis() - lastFlowReportMs >= 1000) {
    lastFlowReportMs = millis();
    const uint32_t p = flowPulses;
    if (p != lastPulses) {
      lastPulses = p;
      Serial.printf("FLOW TEST  %.2f L  (%lu pulses)\n", p / PULSES_PER_L, (unsigned long)p);
      flowLine();
    }
  }

  autoTick();
  if (active >= 0 && millis() - startMs >= RUN_TIMEOUT_MS) stop("TIMEOUT - auto stop");
  slow("loop total", loopStart);
}
