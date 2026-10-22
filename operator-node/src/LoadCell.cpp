// ============================================================================
// LoadCell.cpp — S-type load cell on an HX711
//
// See include/LoadCell.h for public API documentation.
// ============================================================================

#include "LoadCell.h"

using namespace LoadCellConfig;

void LoadCell::begin() {
  pinMode(DT_PIN, INPUT_PULLUP);   // unplugged HX711 reads "not ready", not junk
  pinMode(SCK_PIN, OUTPUT);
  digitalWrite(SCK_PIN, LOW);      // SCK high > 60 us powers the HX711 down
  _reads  = 0;
  _sum    = 0;
  _zeroed = false;
  _lastMs = 0;
}

// DT goes LOW when a sample is ready. Then 24 clocks shift it out MSB first;
// a 25th selects channel A, gain 128 for the next one.
long LoadCell::_read() {
  static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
  long v = 0;
  portENTER_CRITICAL(&mux);   // ~60 us; a long SCK-high stall resets the HX711
  for (int i = 0; i < 25; i++) {
    digitalWrite(SCK_PIN, HIGH);
    delayMicroseconds(1);
    if (i < 24) v = (v << 1) | digitalRead(DT_PIN);
    digitalWrite(SCK_PIN, LOW);
    delayMicroseconds(1);
  }
  portEXIT_CRITICAL(&mux);
  if (v & 0x800000) v |= ~0xFFFFFFL;   // 24-bit two's complement
  return v;
}

void LoadCell::poll() {
  if (digitalRead(DT_PIN) != LOW) return;
  _raw    = _read();
  _lastMs = millis();
  if (_reads < SKIP_READS + TARE_READS) {
    if (++_reads > SKIP_READS) _sum += _raw;
    if (_reads == SKIP_READS + TARE_READS) {
      _tare   = _sum / TARE_READS;
      _zeroed = true;
      Serial.printf("[LOAD] HX711 zeroed at raw %ld\r\n", _tare);
    }
  }
}

bool LoadCell::ready() const {
  return _zeroed && millis() - _lastMs <= STALE_MS;
}

float LoadCell::newtons() const {
  return LOAD_SIGN * (_raw - _tare) / LSB_PER_KG * 9.80665f;
}
