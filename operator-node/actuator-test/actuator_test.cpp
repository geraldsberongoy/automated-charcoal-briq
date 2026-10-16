// ============================================================================
// actuator_test.cpp — bench test for both linear actuators (standalone)
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
//   IN5 GPIO23 / IN6 GPIO32 held OFF (solenoid, mixer)
//   Nextion TX -> GPIO16, Nextion RX -> GPIO17, 9600 baud.
//
// Control: tap a button on the Nextion to run that move, tap it again or
// STOP to stop. Serial Monitor (115200) works too: 1/2/3/4 = move, any
// other key = stop. Every move auto-stops after RUN_TIMEOUT_MS.
// ============================================================================

#include <Arduino.h>

#define RXD2 16
#define TXD2 17

// 40 cm at 7 mm/s is ~57 s end to end; built-in end limits stop it anyway
constexpr uint32_t RUN_TIMEOUT_MS = 70000UL;
constexpr uint32_t DEAD_TIME_MS   = 100;

// Relays 1..6 (IN1..IN6). Only 0..3 ever turn on here.
constexpr int RELAY[] = {25, 26, 27, 33, 23, 32};

struct Move { const char *label; int relay; uint16_t x, y; };
constexpr uint16_t BW = 370, BH = 130;
const Move MOVES[] = {
  {"15cm CLOSE (R1)", 0,  20,  70},
  {"15cm OPEN (R2)",  1, 410,  70},
  {"40cm PRESS (R3)", 2,  20, 215},
  {"40cm HOME (R4)",  3, 410, 215},
};
constexpr uint16_t STOP_X = 20, STOP_Y = 360, STOP_W = 760, STOP_H = 100;

int      active  = -1;   // index into MOVES, -1 = all stopped
uint32_t startMs = 0;

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
  for (int i = 0; i < 4; i++) {
    const Move &m = MOVES[i];
    button(m.x, m.y, BW, BH, i == active ? "GREEN" : "GRAY", m.label);
  }
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
  active = -1;
  Serial.printf("STOP (%s)\n", why);
  draw(why);
}

void start(int i) {
  if (i == active) { stop("stopped"); return; }  // same button again = stop
  allOff();
  delay(DEAD_TIME_MS);  // let the previous contact open before reversing
  digitalWrite(RELAY[MOVES[i].relay], LOW);
  active  = i;
  startMs = millis();
  Serial.printf("RUN %s (GPIO%d LOW)\n", MOVES[i].label, RELAY[MOVES[i].relay]);
  draw((String("RUNNING ") + MOVES[i].label).c_str());
}

void onTouch(uint16_t x, uint16_t y) {
  for (int i = 0; i < 4; i++) {
    const Move &m = MOVES[i];
    if (x >= m.x && x < m.x + BW && y >= m.y && y < m.y + BH) { start(i); return; }
  }
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

// ── Arduino ──────────────────────────────────────────────────────────────────

void setup() {
  // Off level BEFORE pinMode so no relay clicks during boot
  for (int pin : RELAY) { digitalWrite(pin, HIGH); pinMode(pin, OUTPUT); }

  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  delay(500);
  Serial.println("\n===== Actuator test: 1/2/3/4 = move, any other key = stop =====");
  initScreen();
}

void loop() {
  pumpRx();

  if (Serial.available()) {
    char c = Serial.read();
    if (c >= '1' && c <= '4') start(c - '1');
    else if (c != '\n' && c != '\r' && active >= 0) stop("stopped");
  }

  if (active >= 0 && millis() - startMs >= RUN_TIMEOUT_MS) stop("TIMEOUT - auto stop");
}
