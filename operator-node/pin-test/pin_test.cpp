// ============================================================================
// pin_test.cpp - perfboard pin check (standalone)
//
// Build/flash with:  pio run -e pin-test -t upload
// Serial Monitor at 115200. Not part of the main firmware.
//
// Tests every ESP32 pin soldered to the perfboard (same map as
// actuator-test and docs/guides/autobriq-perfboard-layout.html):
//   s  short test: bridges between pins, pins shorted to GND or 3V3/5V
//   i  input monitor: touch a jumper from 3V3 or GND to an INPUT edge pin
//   w  walk outputs, safe (relay inputs stay HIGH)
//   W  walk outputs: relay inputs go LOW one at a time (relays CLICK)
//   h  read the HX711 (needs the HX711 wired and powered)
//
// Run 's' with the relay board Dupont jumpers UNPLUGGED. The relay input
// LEDs pull every IN pin up and read as a false short.
// Before 'W', unplug the 12 V side (Busbar 1 feed) so nothing moves.
// ============================================================================

#include <Arduino.h>

struct Pin {
  const char *name;
  uint8_t gpio;
  bool out;       // ESP32 drives this one on the real rig
  bool relay;     // relay input, active LOW
};

const Pin PINS[] = {
  {"RELAY IN1",        25, true,  true},
  {"RELAY IN2",        26, true,  true},
  {"RELAY IN3",        27, true,  true},
  {"RELAY IN4",        33, true,  true},
  {"RELAY IN5",        23, true,  true},
  {"RELAY IN6",        32, true,  true},
  {"HX711 SCK",        18, true,  false},
  {"NEXTION RX (TX2)", 17, true,  false},
  {"HX711 DT",         19, false, false},
  {"NEXTION TX (RX2)", 16, false, false},
  {"FLOW SIG",         13, false, false},
};
constexpr int N = sizeof(PINS) / sizeof(PINS[0]);

bool hasPull(uint8_t g) { return g < 34; }

void releaseAll(uint8_t mode) {
  for (int i = 0; i < N; i++) {
    pinMode(PINS[i].gpio, hasPull(PINS[i].gpio) ? mode : INPUT);
  }
}

void help() {
  Serial.println();
  Serial.println("pin test: s short | i inputs | w walk safe | W walk relays | h hx711 | ? help");
  Serial.println("s needs the relay board jumpers unplugged. W clicks relays: unplug the 12 V side first.");
}

// Drive each output pin HIGH in turn; anything else that follows it is bridged to it.
// Then check each pin alone against a pull-up and a pull-down for a short to a rail.
void shortTest() {
  int faults = 0;
  Serial.println("-- bridge test (drive one pin HIGH, read the rest)");
  for (int d = 0; d < N; d++) {
    if (!PINS[d].out) continue;
    releaseAll(INPUT_PULLDOWN);
    pinMode(PINS[d].gpio, OUTPUT);
    digitalWrite(PINS[d].gpio, HIGH);
    delay(5);
    for (int o = 0; o < N; o++) {
      if (o == d) continue;
      if (digitalRead(PINS[o].gpio)) {
        Serial.printf("FAIL  %s (GPIO%d) is bridged to %s (GPIO%d)%s\n",
                      PINS[d].name, PINS[d].gpio, PINS[o].name, PINS[o].gpio,
                      hasPull(PINS[o].gpio) ? "" : "  [GPIO34 floats, recheck by eye]");
        faults++;
      }
    }
  }
  Serial.println("-- rail test (each pin alone, pull-up then pull-down)");
  for (int i = 0; i < N; i++) {
    if (!hasPull(PINS[i].gpio)) {
      Serial.printf("skip  %s (GPIO%d) has no internal pull: meter it to GND and 3V3\n", PINS[i].name, PINS[i].gpio);
      continue;
    }
    releaseAll(INPUT);
    pinMode(PINS[i].gpio, INPUT_PULLUP);   delay(5); bool up   = digitalRead(PINS[i].gpio);
    pinMode(PINS[i].gpio, INPUT_PULLDOWN); delay(5); bool down = digitalRead(PINS[i].gpio);
    if (!up)        { Serial.printf("FAIL  %s (GPIO%d) reads LOW on a pull-up: shorted to GND\n", PINS[i].name, PINS[i].gpio); faults++; }
    else if (down)  { Serial.printf("FAIL  %s (GPIO%d) reads HIGH on a pull-down: shorted to 3V3/5V\n", PINS[i].name, PINS[i].gpio); faults++; }
    else            Serial.printf("ok    %s (GPIO%d)\n", PINS[i].name, PINS[i].gpio);
  }
  releaseAll(INPUT);
  Serial.println(faults ? "RESULT: faults found, fix them before plugging modules in" : "RESULT: no bridges, no rail shorts");
  Serial.println("Software cannot see an open or swapped wire: beep each ESP32 pin to its edge pin with a meter.");
}

// Touch a jumper from 3V3 (HIGH) or GND (LOW) to each INPUT edge pin; prints on change.
void inputMonitor() {
  Serial.println("-- input monitor 20 s: touch 3V3 then GND to HX711 DT, NEXTION TX, FLOW SIG");
  int last[N];
  for (int i = 0; i < N; i++) {
    last[i] = -1;
    if (!PINS[i].out) pinMode(PINS[i].gpio, hasPull(PINS[i].gpio) ? INPUT_PULLUP : INPUT);
  }
  uint32_t end = millis() + 20000;
  while (millis() < end) {
    for (int i = 0; i < N; i++) {
      if (PINS[i].out) continue;
      int v = digitalRead(PINS[i].gpio);
      delay(20);                         // debounce: must read the same twice
      if (v != digitalRead(PINS[i].gpio) || v == last[i]) continue;
      last[i] = v;
      Serial.printf("%s (GPIO%d) -> %s\n", PINS[i].name, PINS[i].gpio, v ? "HIGH" : "LOW");
    }
  }
  releaseAll(INPUT);
  Serial.println("done. A pin that never changed has a bad joint or the jumper missed it.");
}

void walk(bool withRelays) {
  Serial.println(withRelays ? "-- walk with relays (each IN goes LOW 1.5 s: relay clicks, LED on)"
                            : "-- walk, safe (relay INs stay HIGH, only SCK and TX2 toggle)");
  releaseAll(INPUT);
  for (int i = 0; i < N; i++) {
    if (!PINS[i].out) continue;
    if (PINS[i].relay && !withRelays) continue;
    pinMode(PINS[i].gpio, OUTPUT);
    digitalWrite(PINS[i].gpio, PINS[i].relay ? HIGH : LOW);   // relays idle HIGH (off)
    delay(100);
    Serial.printf("%s (GPIO%d): %s for 1.5 s - meter at the edge pin\n",
                  PINS[i].name, PINS[i].gpio, PINS[i].relay ? "LOW" : "HIGH");
    digitalWrite(PINS[i].gpio, PINS[i].relay ? LOW : HIGH);
    delay(1500);
    digitalWrite(PINS[i].gpio, PINS[i].relay ? HIGH : LOW);
    delay(300);
    pinMode(PINS[i].gpio, INPUT);
  }
  Serial.println("done. Each edge pin should have shown 3.3 V (or 0 V for a relay IN) on cue.");
}

void hx711() {
  const uint8_t DT = 19, SCK = 18;
  pinMode(DT, INPUT);
  pinMode(SCK, OUTPUT);
  digitalWrite(SCK, LOW);
  uint32_t t = millis();
  while (digitalRead(DT) && millis() - t < 1000) delay(1);
  if (digitalRead(DT)) {
    Serial.println("FAIL  DT never went LOW: HX711 unpowered (3V3, GND) or DT/SCK on the wrong pins");
    return;
  }
  int32_t v = 0;
  for (int i = 0; i < 24; i++) {
    digitalWrite(SCK, HIGH); delayMicroseconds(2);
    v = (v << 1) | digitalRead(DT);
    digitalWrite(SCK, LOW);  delayMicroseconds(2);
  }
  digitalWrite(SCK, HIGH); delayMicroseconds(2); digitalWrite(SCK, LOW);   // 25th pulse: gain 128
  if (v & 0x800000) v |= 0xFF000000;
  Serial.printf("ok    HX711 raw = %ld (all zeros or all ones means the load cell wires are off)\n", (long)v);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  releaseAll(INPUT);
  help();
}

void loop() {
  if (!Serial.available()) return;
  char c = Serial.read();
  switch (c) {
    case 's': shortTest(); break;
    case 'i': inputMonitor(); break;
    case 'w': walk(false);
              Serial.println("(W, uppercase, also clicks the relays)"); break;
    case 'W': walk(true); break;
    case 'h': hx711(); break;
    case '?': help(); break;
    default: break;
  }
}
