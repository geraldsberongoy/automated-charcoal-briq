// ============================================================================
// main.cpp — ESP32 Operator Node — Executive Entry Point
//
// This file is intentionally thin. It acts as an executive summary of the
// firmware — it delegates ALL domain logic to the relevant module.
//
// Modules:
//   NetworkManager — SoftAP, DNS hijacking, HTTP captive portal
//
// Adding features: create a new module under include/ + src/, then
// instantiate and call it here. Do NOT add business logic to this file.
// ============================================================================

#include <Arduino.h>
#include "NetworkManager.h"

// ── Module instantiation ─────────────────────────────────────────────────────
// NetworkManager owns its own DNSServer and WebServer instances internally.
// No global raw pointers or naked singletons — the object is stack-allocated
// in the translation unit that owns the lifetime (.cpp → linker section).
static NetworkManager networkManager;

// ── Periodic diagnostic state ─────────────────────────────────────────────────
static uint32_t _lastStatusPrintMs  = 0;
static uint32_t _lastClientCountMs  = 0;
static uint8_t  _prevClientCount    = 0;

constexpr uint32_t STATUS_INTERVAL_MS      = 30000UL;  // Print status every 30 s
constexpr uint32_t CLIENT_CHECK_INTERVAL_MS = 5000UL;  // Check client count every 5 s

// ── setup() ──────────────────────────────────────────────────────────────────

void setup() {
  // Serial is initialised inside NetworkManager::begin() if not already started.
  // To override baud rate or start earlier, uncomment the line below:
  // Serial.begin(115200);

  // Initialise the network stack — SoftAP + DNS + HTTP
  bool networkOK = networkManager.begin();

  if (!networkOK) {
    // Fatal: networking failed — blink the built-in LED rapidly and halt.
    Serial.println(F("[FATAL] Network initialisation failed. System halted."));
    pinMode(LED_BUILTIN, OUTPUT);
    while (true) {
      digitalWrite(LED_BUILTIN, HIGH); delay(100);
      digitalWrite(LED_BUILTIN, LOW);  delay(100);
    }
  }

  Serial.println(F("[MAIN] setup() complete — entering loop()"));
}

// ── loop() ───────────────────────────────────────────────────────────────────

void loop() {
  const uint32_t now = millis();

  // ── 1. Pump network services (must be called every iteration) ─────────────
  networkManager.handle();

  // ── 2. Client count change detection ─────────────────────────────────────
  if ((now - _lastClientCountMs) >= CLIENT_CHECK_INTERVAL_MS) {
    _lastClientCountMs = now;

    uint8_t currentCount = networkManager.getClientCount();
    if (currentCount != _prevClientCount) {
      if (currentCount > _prevClientCount) {
        Serial.printf("[MAIN] Client JOINED  — total clients: %d\r\n", currentCount);
      } else {
        Serial.printf("[MAIN] Client LEFT    — total clients: %d\r\n", currentCount);
      }
      _prevClientCount = currentCount;
    }
  }

  // ── 3. Periodic heartbeat / status diagnostic ─────────────────────────────
  if ((now - _lastStatusPrintMs) >= STATUS_INTERVAL_MS) {
    _lastStatusPrintMs = now;

    Serial.println(F("[MAIN] ─── Heartbeat ────────────────────────────────────────"));
    Serial.printf ("[MAIN]  Uptime        : %lu ms\r\n",     now);
    Serial.printf ("[MAIN]  Free Heap     : %u bytes\r\n",   ESP.getFreeHeap());
    Serial.printf ("[MAIN]  Min Free Heap : %u bytes\r\n",   ESP.getMinFreeHeap());
    Serial.printf ("[MAIN]  Active Clients: %d\r\n",         networkManager.getClientCount());
    Serial.printf ("[MAIN]  CPU Freq      : %u MHz\r\n",     getCpuFrequencyMhz());
    Serial.println(F("[MAIN] ─────────────────────────────────────────────────────────"));
  }
}
