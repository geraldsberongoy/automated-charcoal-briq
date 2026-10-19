// ============================================================================
// flow_test.cpp - YF-S201 / YFS-201G1 flow sensor calibration (standalone)
//
// Build/flash with:  pio run -e flow-test -t upload
// Serial Monitor at 115200. Not part of the main firmware.
//
// Wiring: sensor red -> 5 V, black -> GND, yellow -> 10k -> GPIO34
//         (docs/guides/autobriq-flow-sensor-yfs201.html).
//         Optional: valve relay IN6 on GPIO32 for the 'd' dose command.
//
// Calibrate:  z  zero | run water into a jug | m <litres in the jug>
//             repeat 2 or 3 times | a  average | s  save and use it
// Commands:   z zero   m <L> record a run   a average   s save average
//             u back to 450   d <L> dose with the valve   x stop valve
//             r raw GPIO34 mV for 20 s   ? help
//
// Unlike actuator-test, pulses closer than MIN_GAP_US apart are dropped as
// noise (the real sensor tops out near 230 Hz), and the count is reported
// as "rej" so you can see how much noise was filtered.
// ============================================================================

#include <Arduino.h>
#include <Preferences.h>

constexpr int      FLOW_PIN      = 34;
constexpr int      VALVE_PIN     = 32;     // relay IN6, active LOW
constexpr float    DEFAULT_PPL   = 450.0f; // datasheet pulses per litre
constexpr uint32_t MIN_GAP_US    = 2000;   // 500 Hz ceiling, well above 230 Hz
constexpr uint32_t DOSE_MAX_MS   = 90000;  // valve shuts after this, no matter what
constexpr int      MAX_RUNS      = 10;

volatile uint32_t pulses = 0, rejected = 0, lastUs = 0;

void IRAM_ATTR onPulse() {
  uint32_t now = micros();
  if (now - lastUs < MIN_GAP_US) { rejected++; return; }
  lastUs = now;
  pulses++;
}

Preferences prefs;
float ppl = DEFAULT_PPL;
float runs[MAX_RUNS];
int   nRuns = 0;

bool     dosing = false;
uint32_t dosePulses = 0, doseStartMs = 0;
uint32_t rawUntilMs = 0;

void valve(bool open) {
  digitalWrite(VALVE_PIN, open ? LOW : HIGH);
  if (!open) dosing = false;
}

void zero() {
  noInterrupts(); pulses = 0; rejected = 0; interrupts();
  Serial.println("zeroed");
}

float average(float *spreadPct) {
  float sum = 0, lo = runs[0], hi = runs[0];
  for (int i = 0; i < nRuns; i++) { sum += runs[i]; lo = min(lo, runs[i]); hi = max(hi, runs[i]); }
  float avg = sum / nRuns;
  *spreadPct = 100.0f * (hi - lo) / avg;
  return avg;
}

void showAverage() {
  if (!nRuns) { Serial.println("no runs yet: z, run water, m <litres>"); return; }
  float spread, avg = average(&spread);
  Serial.printf("%d run(s): average %.1f pulses/L, spread %.1f%%\n", nRuns, avg, spread);
  if (nRuns < 2)      Serial.println("do at least 2 or 3 runs");
  else if (spread > 5) Serial.println("spread over 5%: pour at a steady rate, read the jug level at eye height, repeat");
  else                 Serial.println("good. s saves it");
}

void handle(String line) {
  line.trim();
  if (!line.length()) return;
  char c = line[0];
  float arg = line.substring(1).toFloat();
  uint32_t p = pulses;
  switch (c) {
    case 'z': zero(); break;
    case 'm':
      if (arg < 0.05f || p < 20) { Serial.println("usage: m <litres in the jug>, after at least 20 pulses"); break; }
      if (nRuns == MAX_RUNS) { Serial.println("10 runs is plenty: a, then s"); break; }
      runs[nRuns++] = p / arg;
      Serial.printf("run %d: %lu pulses for %.2f L = %.1f pulses/L\n", nRuns, (unsigned long)p, arg, runs[nRuns - 1]);
      zero();
      break;
    case 'a': showAverage(); break;
    case 's': {
      if (!nRuns) { Serial.println("no runs yet"); break; }
      float spread; ppl = average(&spread);
      prefs.putFloat("ppl", ppl);
      Serial.printf("saved. In actuator_test.cpp set:  constexpr float PULSES_PER_L = %.1ff;\n", ppl);
      break;
    }
    case 'u': ppl = DEFAULT_PPL; prefs.putFloat("ppl", ppl); nRuns = 0; Serial.println("back to 450, runs cleared"); break;
    case 'd':
      if (arg < 0.05f || arg > 5.0f) { Serial.println("usage: d <litres>, 0.05 to 5"); break; }
      zero();
      dosePulses = (uint32_t)(arg * ppl);
      doseStartMs = millis();
      valve(true);
      dosing = true;
      Serial.printf("valve open, closing at %lu pulses (%.2f L at %.1f pulses/L). x stops\n", (unsigned long)dosePulses, arg, ppl);
      break;
    case 'x': valve(false); Serial.println("valve closed"); break;
    case 'r': rawUntilMs = millis() + 20000; Serial.println("RAW GPIO34 for 20 s"); break;
    default:
      Serial.printf("z zero | m <L> record | a average | s save | u reset | d <L> dose | x stop | r raw   (now %.1f pulses/L)\n", ppl);
  }
}

void setup() {
  digitalWrite(VALVE_PIN, HIGH);          // valve closed before the pin becomes an output
  pinMode(VALVE_PIN, OUTPUT);
  Serial.begin(115200);
  delay(500);
  prefs.begin("flow", false);
  ppl = prefs.getFloat("ppl", DEFAULT_PPL);
  pinMode(FLOW_PIN, INPUT);               // no pull-up on GPIO34; sensor drives it
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), onPulse, FALLING);
  Serial.printf("\n===== flow calibration, now %.1f pulses/L. Type ? for commands =====\n", ppl);
}

String line;
uint32_t lastPrintMs = 0, lastPulses = 0, lastIdleMs = 0;

void loop() {
  while (Serial.available()) {
    char ch = Serial.read();
    if (ch == '\n' || ch == '\r') { handle(line); line = ""; }
    else line += ch;
  }

  uint32_t p = pulses;
  if (dosing && (p >= dosePulses || millis() - doseStartMs > DOSE_MAX_MS)) {
    valve(false);
    Serial.printf("dose done: %lu pulses = %.3f L by the sensor%s. Measure the jug, then m <litres>\n",
                  (unsigned long)p, p / ppl, p >= dosePulses ? "" : " (TIMED OUT, no flow?)");
  }

  if (millis() < rawUntilMs) {
    static uint32_t rawMs = 0;
    if (millis() - rawMs >= 100) { rawMs = millis(); Serial.printf("RAW GPIO34 %4d mV\n", analogReadMilliVolts(FLOW_PIN)); }
    return;
  }

  uint32_t now = millis();
  if (now - lastPrintMs >= 500) {
    float dt = (now - lastPrintMs) / 1000.0f;
    uint32_t d = p - lastPulses;
    lastPrintMs = now; lastPulses = p;
    if (d || now - lastIdleMs > 5000) {
      lastIdleMs = now;
      Serial.printf("%6lu pulses  %.3f L  %5.1f Hz  %.2f L/min  (rej %lu)\n",
                    (unsigned long)p, p / ppl, d / dt, (d / dt) / ppl * 60.0f, (unsigned long)rejected);
    }
  }
}
