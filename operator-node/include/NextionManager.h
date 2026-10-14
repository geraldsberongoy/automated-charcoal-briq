#pragma once
// ============================================================================
// NextionManager.h — Nextion HMI UART Transport
//
// This module owns the UART link to the Nextion touchscreen:
//   1. Serial2 bring-up (GPIO16 RX / GPIO17 TX), auto-switch to 115200 bps
//   2. Outbound commands terminated by 0xFF 0xFF 0xFF
//   3. Inbound frame parsing (touch coordinates, page changes, errors)
//
// It knows nothing about layout — NextionDashboard draws on top of it.
//
// Usage:
//   NextionManager hmi;
//   hmi.begin();                     // call once in setup()
//   hmi.handle();                    // call every loop iteration
//   hmi.sendCommand("cls BLACK");
//   if (hmi.takeTouch(x, y)) { ... }
// ============================================================================

#include <Arduino.h>

namespace NextionConfig {
  // Wiring: Nextion TX → ESP32 RX pin, Nextion RX → ESP32 TX pin
  constexpr int8_t   RX_PIN    = 16;
  constexpr int8_t   TX_PIN    = 17;

  // Nextion factory default, and the faster rate we switch to at boot.
  // The switch is runtime-only: the display reverts to 9600 on power-off.
  constexpr uint32_t BOOT_BAUD = 9600;
  constexpr uint32_t RUN_BAUD  = 115200;

  // TX ring buffer so a full-screen redraw never blocks loop()
  constexpr size_t   TX_BUFFER = 2048;

  // Flow control: bkcmd=3 makes the display ack every command. At most
  // MAX_INFLIGHT unacked commands are outstanding, so we never overrun its
  // 1 KB input buffer while it is busy drawing.
  constexpr uint8_t  MAX_INFLIGHT   = 4;
  constexpr uint32_t ACK_TIMEOUT_MS = 250UL;

  // Keepalive: ping every PING_INTERVAL_MS, resync after LINK_TIMEOUT_MS silence
  constexpr uint32_t PING_INTERVAL_MS = 2000UL;
  constexpr uint32_t LINK_TIMEOUT_MS  = 5000UL;

  // Longest inbound frame we buffer before discarding (excl. terminator)
  constexpr size_t   RX_MAX    = 64;
}


// ── NextionManager Class ─────────────────────────────────────────────────────

class NextionManager {
public:
  NextionManager();

  /**
   * @brief  Open Serial2, move the display to RUN_BAUD, enable touch reports.
   */
  void begin();

  /**
   * @brief  Drain and parse inbound bytes. Call every loop() iteration.
   */
  void handle();

  /**
   * @brief  Send a raw Nextion instruction (terminator appended).
   *         Blocks briefly when MAX_INFLIGHT commands are still unacked.
   */
  void sendCommand(const char *cmd);

  /**
   * @brief  printf-style sendCommand().
   */
  void sendf(const char *fmt, ...) __attribute__((format(printf, 2, 3)));

  /**
   * @brief  Pop the latest touch-press coordinate (from sendxy=1 reports).
   * @return true if a press arrived since the last call.
   */
  bool takeTouch(uint16_t &x, uint16_t &y);

  /**
   * @brief  True once after the display rebooted or the link was resynced — the caller
   *         must re-initialise and redraw everything.
   */
  bool takeRestart();

  /**
   * @brief  millis() of the last frame received from the display.
   */
  uint32_t lastRxMs() const { return _lastRxMs; }

private:
  uint8_t  _rxBuf[NextionConfig::RX_MAX];
  size_t   _rxLen;
  uint8_t  _ffCount;    // consecutive 0xFF bytes seen (3 = end of frame)

  bool     _touchPending;
  uint16_t _touchX;
  uint16_t _touchY;
  bool     _restartPending;
  uint32_t _lastRxMs;   // millis() of the last frame from the display
  uint32_t _lastPingMs;
  uint8_t  _inflight;   // commands sent but not yet acked

  /**
   * @brief  Parse whatever bytes have arrived (no keepalive, no sending).
   */
  void _pumpRx();

  /**
   * @brief  (Re)negotiate RUN_BAUD and re-enable touch reports.
   */
  void _syncLink();

  /**
   * @brief  Decode one complete inbound frame.
   */
  void _handleFrame(const uint8_t *data, size_t len);
};
