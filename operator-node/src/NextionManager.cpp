// ============================================================================
// NextionManager.cpp — Nextion HMI UART Transport Implementation
//
// See include/NextionManager.h for public API documentation.
// ============================================================================

#include "NextionManager.h"
#include <stdarg.h>

// ── Constructor ──────────────────────────────────────────────────────────────

NextionManager::NextionManager()
  : _rxLen(0),
    _ffCount(0),
    _touchPending(false),
    _touchX(0),
    _touchY(0),
    _restartPending(false),
    _lastRxMs(0),
    _lastPingMs(0),
    _inflight(0)
{
  // No hardware access — deferred to begin().
}

// ── Public: begin() ──────────────────────────────────────────────────────────

void NextionManager::begin() {
  // Buffer size must be set before begin() to take effect
  Serial2.setTxBufferSize(NextionConfig::TX_BUFFER);
  Serial2.begin(NextionConfig::BOOT_BAUD, SERIAL_8N1,
                NextionConfig::RX_PIN, NextionConfig::TX_PIN);
  _syncLink();

  Serial.printf("[HMI ] Nextion UART on Serial2 (RX=%d, TX=%d, %lu bps)\r\n",
                NextionConfig::RX_PIN, NextionConfig::TX_PIN,
                (unsigned long)NextionConfig::RUN_BAUD);
}

// ── Public: handle() ─────────────────────────────────────────────────────────

void NextionManager::handle() {
  const uint32_t now = millis();

  // Keepalive: the display answers `sendme` with its page number. Silence
  // means it power-cycled (back at 9600) or was unplugged — resync.
  if ((now - _lastPingMs) >= NextionConfig::PING_INTERVAL_MS) {
    _lastPingMs = now;
    if ((now - _lastRxMs) >= NextionConfig::LINK_TIMEOUT_MS) {
      Serial.println(F("[HMI ] No reply from display — resyncing link"));
      _syncLink();
      _lastRxMs = now;           // give it a full timeout before retrying
      _restartPending = true;    // caller redraws everything
    }
    sendCommand("sendme");
  }

  _pumpRx();
}

// ── Private: _pumpRx() ───────────────────────────────────────────────────────

void NextionManager::_pumpRx() {
  while (Serial2.available()) {
    uint8_t b = static_cast<uint8_t>(Serial2.read());

    if (b == 0xFF) {
      if (++_ffCount == 3) {
        _handleFrame(_rxBuf, _rxLen);
        _rxLen   = 0;
        _ffCount = 0;
      }
      continue;
    }

    // 0xFF bytes that were not a terminator belong to the payload
    while (_ffCount > 0 && _rxLen < NextionConfig::RX_MAX) {
      _rxBuf[_rxLen++] = 0xFF;
      --_ffCount;
    }
    _ffCount = 0;

    if (_rxLen < NextionConfig::RX_MAX) {
      _rxBuf[_rxLen++] = b;
    } else {
      // Overlong frame — drop it and resync on the next terminator
      _rxLen = 0;
    }
  }
}

// ── Public: sendCommand() / sendf() ──────────────────────────────────────────

void NextionManager::sendCommand(const char *cmd) {
  static const uint8_t TERMINATOR[3] = {0xFF, 0xFF, 0xFF};

  // Flow control — skipped while the link is down so a missing display
  // never stalls loop()
  const bool linkUp = (millis() - _lastRxMs) < NextionConfig::LINK_TIMEOUT_MS;
  if (linkUp && _inflight >= NextionConfig::MAX_INFLIGHT) {
    const uint32_t start = millis();
    while (_inflight >= NextionConfig::MAX_INFLIGHT) {
      _pumpRx();
      if ((millis() - start) >= NextionConfig::ACK_TIMEOUT_MS) {
        Serial.println(F("[HMI ] Ack timeout — display busy or replies lost"));
        _inflight = 0;   // lost acks — don't wait forever
        break;
      }
      delay(1);
    }
  }

  Serial2.print(cmd);
  Serial2.write(TERMINATOR, sizeof(TERMINATOR));
  if (linkUp) ++_inflight;
}

void NextionManager::sendf(const char *fmt, ...) {
  char cmd[160];
  va_list args;
  va_start(args, fmt);
  vsnprintf(cmd, sizeof(cmd), fmt, args);
  va_end(args);
  sendCommand(cmd);
}

// ── Public: takeTouch() / takeRestart() ──────────────────────────────────────

bool NextionManager::takeTouch(uint16_t &x, uint16_t &y) {
  if (!_touchPending) return false;
  _touchPending = false;
  x = _touchX;
  y = _touchY;
  return true;
}

bool NextionManager::takeRestart() {
  const bool pending = _restartPending;
  _restartPending = false;
  return pending;
}

// ── Private: _syncLink() ─────────────────────────────────────────────────────

void NextionManager::_syncLink() {
  // The display is at 9600 after power-on, but still at RUN_BAUD if only the
  // ESP32 rebooted. Ask for the switch at 9600 (garbage if already switched,
  // which the display ignores), then continue at RUN_BAUD either way.
  Serial2.updateBaudRate(NextionConfig::BOOT_BAUD);
  sendCommand("");
  sendf("baud=%lu", (unsigned long)NextionConfig::RUN_BAUD);
  Serial2.flush();
  delay(100);
  Serial2.updateBaudRate(NextionConfig::RUN_BAUD);
  delay(50);
  while (Serial2.available()) Serial2.read();  // drop replies from the switch
  _rxLen    = 0;
  _ffCount  = 0;
  _inflight = 0;

  sendCommand("");          // flush any partial command in the display's parser
  sendCommand("bkcmd=3");   // ack every command — drives flow control
  sendCommand("sendxy=1");  // report raw touch coordinates (0x67 frames)
}

// ── Private: _handleFrame() ──────────────────────────────────────────────────

void NextionManager::_handleFrame(const uint8_t *data, size_t len) {
  if (len == 0) return;
  _lastRxMs = millis();

  // Every reply except unsolicited events (touch 0x65/0x67, boot 0x88) and
  // the startup frame answers exactly one command
  const bool event = data[0] == 0x65 || data[0] == 0x67 || data[0] == 0x88 ||
                     (data[0] == 0x00 && len >= 3 && data[1] == 0x00 && data[2] == 0x00);
  if (!event && _inflight > 0) --_inflight;

  switch (data[0]) {
    case 0x67:  // Touch coordinate: xH xL yH yL state
      if (len >= 6 && data[5] == 1) {
        _touchX = (data[1] << 8) | data[2];
        _touchY = (data[3] << 8) | data[4];
        _touchPending = true;
      }
      break;
    case 0x65:  // Component touch event — unused, we hit-test coordinates
    case 0x66:  // Current page (reply to `sendme`)
    case 0x01:  // Instruction successful (bkcmd=3)
      break;
    case 0x88:  // Ready after power-on
      Serial.println(F("[HMI ] Display (re)booted"));
      _restartPending = true;
      break;
    case 0x00:  // Startup frame (00 00 00) or invalid instruction
      if (len >= 3 && data[1] == 0x00 && data[2] == 0x00) {
        _restartPending = true;
      } else {
        Serial.println(F("[HMI ] ERR invalid instruction"));
      }
      break;
    case 0x1A:
      Serial.println(F("[HMI ] ERR invalid variable name/attribute"));
      break;
    case 0x24:
      Serial.println(F("[HMI ] ERR display serial buffer overflow"));
      break;
    default:
      Serial.printf("[HMI ] Frame 0x%02X (%u bytes)\r\n", data[0], (unsigned)len);
      break;
  }
}
