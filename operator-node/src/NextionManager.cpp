// ============================================================================
// NextionManager.cpp — Nextion HMI UART Driver Implementation
//
// See include/NextionManager.h for public API documentation.
// ============================================================================

#include "NextionManager.h"

// ── Constructor ──────────────────────────────────────────────────────────────

NextionManager::NextionManager()
  : _rxLen(0),
    _ffCount(0),
    _lastRxMs(0)
{
  // No hardware access — deferred to begin().
}

// ── Public: begin() ──────────────────────────────────────────────────────────

void NextionManager::begin() {
  // Buffer size must be set before begin() to take effect
  Serial2.setTxBufferSize(NextionConfig::TX_BUFFER);
  Serial2.begin(NextionConfig::BAUD_RATE, SERIAL_8N1,
                NextionConfig::RX_PIN, NextionConfig::TX_PIN);

  Serial.printf("[HMI ] Nextion UART on Serial2 (RX=%d, TX=%d, %lu bps)\r\n",
                NextionConfig::RX_PIN, NextionConfig::TX_PIN,
                (unsigned long)NextionConfig::BAUD_RATE);

  // Flush any partial command left in the display's parser from a prior boot
  sendCommand("");
  sendCommand("bkcmd=1");   // reply to every command with success/failure code
  sendCommand("page 0");
  sendCommand("dim=100");
}

// ── Public: handle() ─────────────────────────────────────────────────────────

void NextionManager::handle() {
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

// ── Public: setText() / setValue() / sendCommand() ───────────────────────────

void NextionManager::setText(const char *component, const char *value) {
  char safe[64];
  size_t i = 0;
  for (; value[i] != '\0' && i < sizeof(safe) - 1; ++i) {
    safe[i] = (value[i] == '"') ? '\'' : value[i];
  }
  safe[i] = '\0';

  char cmd[96];
  snprintf(cmd, sizeof(cmd), "%s.txt=\"%s\"", component, safe);
  sendCommand(cmd);
}

void NextionManager::setValue(const char *component, int32_t value) {
  char cmd[48];
  snprintf(cmd, sizeof(cmd), "%s.val=%ld", component, (long)value);
  sendCommand(cmd);
}

void NextionManager::sendCommand(const char *cmd) {
  static const uint8_t TERMINATOR[3] = {0xFF, 0xFF, 0xFF};
  Serial2.print(cmd);
  Serial2.write(TERMINATOR, sizeof(TERMINATOR));
}

// ── Public: showStatus() ─────────────────────────────────────────────────────

void NextionManager::showStatus(uint32_t uptimeMs, uint8_t clients,
                                const char *ip, uint32_t freeHeap) {
  char buf[32];
  const uint32_t s = uptimeMs / 1000UL;

  snprintf(buf, sizeof(buf), "Up %02lu:%02lu:%02lu",
           (unsigned long)(s / 3600UL), (unsigned long)((s / 60UL) % 60UL),
           (unsigned long)(s % 60UL));
  setText("t0", buf);

  snprintf(buf, sizeof(buf), "Clients: %u", (unsigned)clients);
  setText("t1", buf);

  snprintf(buf, sizeof(buf), "IP: %s", ip);
  setText("t2", buf);

  snprintf(buf, sizeof(buf), "Heap: %lu KB", (unsigned long)(freeHeap / 1024UL));
  setText("t3", buf);
}

// ── Private: _handleFrame() ──────────────────────────────────────────────────

void NextionManager::_handleFrame(const uint8_t *data, size_t len) {
  if (len == 0) return;
  _lastRxMs = millis();

  switch (data[0]) {
    case 0x01:  // Instruction successful (bkcmd=1) — too chatty to log
      break;
    case 0x65:  // Touch event: page, component, press(1)/release(0)
      if (len >= 4) {
        Serial.printf("[HMI ] Touch: page=%u id=%u %s\r\n",
                      data[1], data[2], data[3] ? "press" : "release");
      }
      break;
    case 0x66:  // Current page (reply to `sendme`)
      if (len >= 2) Serial.printf("[HMI ] Page: %u\r\n", data[1]);
      break;
    case 0x70:  // String data (from `get` or `prints`), e.g. "state=2"
      Serial.printf("[HMI ] String: %.*s\r\n", (int)(len - 1), data + 1);
      break;
    case 0x00:  // Startup frame, or invalid instruction (bkcmd=1)
      Serial.println(F("[HMI ] Startup / invalid instruction"));
      break;
    case 0x88:  // Ready after power-on
      Serial.println(F("[HMI ] Display ready"));
      break;
    case 0x02:
      Serial.println(F("[HMI ] ERR invalid component ID"));
      break;
    case 0x1A:
      Serial.println(F("[HMI ] ERR invalid variable name/attribute (check page0 has t0..t3)"));
      break;
    default:
      Serial.printf("[HMI ] Frame 0x%02X (%u bytes)\r\n", data[0], (unsigned)len);
      break;
  }
}
