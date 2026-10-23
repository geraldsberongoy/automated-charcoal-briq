#pragma once
// ============================================================================
// LoadCell.h — S-type load cell on an HX711 (bit-banged, polled)
//
// Same wiring and scale as the bench sketch actuator-test: DT GPIO19, SCK GPIO18,
// HX711 on 3V3. poll() is non-blocking: it only reads when the HX711 says a
// sample is ready (~10 per second). The first ~3 s after begin() zero the cell,
// so keep the press unloaded at power-up.
// ============================================================================

#include <Arduino.h>

namespace LoadCellConfig {
  constexpr int8_t  DT_PIN        = 19;
  constexpr int8_t  SCK_PIN       = 18;
  // Measured with a 2 kg weight on 3V3 excitation: (33730 - 5860) / 2 kg.
  // Re-check with a second weight: new = (raw - tare) / kg on the scale.
  constexpr float   LSB_PER_KG    = 13935.0f;
  // Pressing read negative on the rig (signal pair reversed for how the cell is
  // mounted). Set to 1 if the white and green wires are swapped.
  constexpr float   LOAD_SIGN     = -1.0f;
  constexpr uint8_t SKIP_READS    = 20;     // power-up jump, discarded
  constexpr uint8_t TARE_READS    = 10;     // averaged into the zero
  constexpr uint32_t STALE_MS     = 2000;   // no sample this long = HX711 lost
}

class LoadCell {
public:
  void begin();

  /**
   * @brief  Read one sample if the HX711 has one. Call every few ms.
   */
  void poll();

  /**
   * @brief  True once zeroed and still answering.
   */
  bool ready() const;

  /**
   * @brief  Load in newtons (negative = pulling). Only valid while ready().
   */
  float newtons() const;

private:
  long     _raw    = 0;
  long     _tare   = 0;
  long     _sum    = 0;
  uint8_t  _reads  = 0;
  uint32_t _lastMs = 0;
  bool     _zeroed = false;

  long _read();
};
