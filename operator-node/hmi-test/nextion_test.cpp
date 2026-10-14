// ============================================================================
// nextion_test.cpp — Nextion NX8048T070 bring-up test (standalone)
//
// Build/flash with:  pio run -e nextion-test -t upload
// Not part of the main firmware. Runs the handoff test plan:
//   pixel test → 60 s full-white power stress → touch coordinate echo.
//
// Wiring: Nextion TX -> GPIO16, Nextion RX -> GPIO17, shared GND,
//         Nextion on its own 5V supply. Serial Monitor at 115200.
// ============================================================================

#include <Arduino.h>

#define RXD2 16
#define TXD2 17

// Lower to 30 and shorten STRESS_MS on a weak 5V supply
constexpr int      BRIGHTNESS = 100;
constexpr uint32_t STRESS_MS  = 60000UL;

void cmd(const String &c) {
  Serial2.print(c);
  Serial2.write(0xFF); Serial2.write(0xFF); Serial2.write(0xFF);
}

uint8_t buf[32];
int len = 0, ffCount = 0;
bool gotReply = false;

void handleFrame() {
  if (!gotReply) { Serial.println("Nextion -> ESP32 link OK"); gotReply = true; }

  if (len == 6 && buf[0] == 0x67) {
    int x = (buf[1] << 8) | buf[2];
    int y = (buf[3] << 8) | buf[4];
    bool pressed = buf[5] == 1;
    Serial.printf("Touch %s  x=%d  y=%d\n", pressed ? "down" : "up  ", x, y);
    if (pressed) {
      cmd("fill " + String(max(0, x - 6)) + "," + String(max(0, y - 6)) + ",12,12,WHITE");
    }
  } else if (len == 1 && buf[0] == 0x01) {
    // command OK reply, ignore
  } else {
    Serial.print("Other data: ");
    for (int i = 0; i < len; i++) Serial.printf("%02X ", buf[i]);
    Serial.println();
  }
}

void pumpRx() {
  while (Serial2.available()) {
    uint8_t b = Serial2.read();
    if (b == 0xFF) {
      if (++ffCount == 3) { handleFrame(); len = 0; ffCount = 0; }
      continue;
    }
    // 0xFF bytes that weren't a terminator belong to the data
    while (ffCount > 0 && len < (int)sizeof(buf)) { buf[len++] = 0xFF; ffCount--; }
    ffCount = 0;
    if (len < (int)sizeof(buf)) buf[len++] = b;
  }
}

// delay() that keeps draining Serial2 so replies are logged as they arrive
void waitMs(uint32_t ms) {
  const uint32_t start = millis();
  while (millis() - start < ms) { pumpRx(); delay(5); }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);   // Nextion default baud
  delay(1000);
  Serial.println("\n===== Nextion test starting =====");

  cmd("");          // flush any partial command in the display's parser
  cmd("bkcmd=3");   // Nextion replies to every command, proves the RX link
  cmd("dim=" + String(BRIGHTNESS));
  waitMs(200);
  if (!gotReply) Serial.println("WARNING: no reply yet (check TX/RX swap, shared GND, baud)");

  // Check 2: pixel test
  const char* colors[] = {"WHITE", "BLACK", "RED", "GREEN", "BLUE"};
  for (auto c : colors) {
    Serial.printf("Pixel test: %s\n", c);
    cmd(String("cls ") + c);
    waitMs(2000);
  }

  // Check 3: power stress
  Serial.printf("Power test: full white, dim=%d for %lu s...\n",
                BRIGHTNESS, (unsigned long)(STRESS_MS / 1000));
  cmd("cls WHITE");
  waitMs(STRESS_MS);  // a brownout shows up as the start banner printing again
  Serial.println("Power test passed");

  // Check 4: touch
  cmd("cls BLACK");
  cmd("sendxy=1");  // send 0x67 frames with touch coordinates
  Serial.println("Touch test: tap the corners and the center");
}

void loop() {
  pumpRx();
}
