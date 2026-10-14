#pragma once
// ============================================================================
// NextionManager.h — Nextion HMI UART Driver Interface
//
// This module owns the UART link to the Nextion touchscreen:
//   1. Serial2 bring-up (GPIO16 RX / GPIO17 TX, 9600 bps)
//   2. Outbound commands terminated by 0xFF 0xFF 0xFF
//   3. Inbound frame parsing (touch events, page changes, errors)
//
// Bring-up test page (see docs/Nextion_HMI_Design.md, "Bring-up Test Page"):
//   page0.t0 → uptime     page0.t1 → connected clients
//   page0.t2 → AP IP      page0.t3 → free heap
//
// Usage (in main.cpp):
//   NextionManager hmi;
//   hmi.begin();                  // call once in setup()
//   hmi.handle();                 // call every loop iteration
//   hmi.setText("t0", "hello");   // push text to a component
// ============================================================================

#include <Arduino.h>

namespace NextionConfig {
  // Wiring: Nextion TX → ESP32 RX pin, Nextion RX → ESP32 TX pin
  constexpr int8_t   RX_PIN    = 16;
  constexpr int8_t   TX_PIN    = 17;

  // Nextion factory default baud rate
  constexpr uint32_t BAUD_RATE = 9600;

  // TX ring buffer so a full status refresh never blocks loop() at 9600 bps
  constexpr size_t   TX_BUFFER = 512;

  // Longest inbound frame we buffer before discarding (excl. terminator)
  constexpr size_t   RX_MAX    = 64;
}


// ── NextionManager Class ─────────────────────────────────────────────────────

class NextionManager {
public:
  NextionManager();

  /**
   * @brief  Open Serial2 and wake the display (page 0, full brightness).
   */
  void begin();

  /**
   * @brief  Drain and parse inbound bytes. Call every loop() iteration.
   */
  void handle();

  /**
   * @brief  Set a text component's .txt attribute, e.g. setText("t0", "42").
   *         Double quotes in the value are replaced with single quotes.
   */
  void setText(const char *component, const char *value);

  /**
   * @brief  Set a numeric attribute, e.g. setValue("j0", 81) → j0.val=81.
   */
  void setValue(const char *component, int32_t value);

  /**
   * @brief  Send a raw Nextion instruction (terminator appended).
   */
  void sendCommand(const char *cmd);

  /**
   * @brief  Push the bring-up status page (page0 t0..t3).
   */
  void showStatus(uint32_t uptimeMs, uint8_t clients,
                  const char *ip, uint32_t freeHeap);

  /**
   * @brief  millis() of the last frame received from the display (0 = never).
   */
  uint32_t lastRxMs() const { return _lastRxMs; }

private:
  uint8_t  _rxBuf[NextionConfig::RX_MAX];
  size_t   _rxLen;
  uint8_t  _ffCount;    // consecutive 0xFF bytes seen (3 = end of frame)
  uint32_t _lastRxMs;

  /**
   * @brief  Decode one complete inbound frame and log it to Serial.
   */
  void _handleFrame(const uint8_t *data, size_t len);
};
