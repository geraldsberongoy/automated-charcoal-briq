// ============================================================================
// main.cpp — ESP32 Operator Node — Executive Entry Point
//
// This file is intentionally thin. It acts as an executive summary of the
// firmware — it delegates ALL domain logic to the relevant module.
//
// Modules:
//   Outputs          — relay/actuator driver (one output at a time)
//   BatchProcess     — batch FSM: setup, 9 automatic steps, unload, stop
//   NetworkManager   — SoftAP, DNS hijacking, HTTP captive portal, WebSocket
//   NextionManager   — Nextion HMI UART transport
//   NextionDashboard — operator dashboard drawn on the Nextion
//
// Adding features: create a new module under include/ + src/, then
// instantiate and call it here. Do NOT add business logic to this file.
// ============================================================================

#include <Arduino.h>
#include "Outputs.h"
#include "BatchProcess.h"
#include "NetworkManager.h"
#include "NextionManager.h"
#include "NextionDashboard.h"

// ── Module instantiation ─────────────────────────────────────────────────────
// NetworkManager owns its own DNSServer and WebServer instances internally.
// No global raw pointers or naked singletons — the object is stack-allocated
// in the translation unit that owns the lifetime (.cpp → linker section).
static Outputs          outputs;
static BatchProcess     process(outputs);
static NetworkManager   networkManager;
static NextionManager   nextion;
static NextionDashboard dashboard(nextion);

// ── Periodic diagnostic state ─────────────────────────────────────────────────
static uint32_t _lastStatusPrintMs  = 0;
static uint32_t _lastClientCountMs  = 0;
static uint32_t _lastRenderMs       = 0;
static uint8_t  _prevClientCount    = 0;

constexpr uint32_t STATUS_INTERVAL_MS      = 30000UL;  // Print status every 30 s
constexpr uint32_t CLIENT_CHECK_INTERVAL_MS = 5000UL;  // Check client count every 5 s
constexpr uint32_t RENDER_INTERVAL_MS       = 500UL;   // HMI refresh

// ── setup() ──────────────────────────────────────────────────────────────────

void setup() {
  // Start Serial before any module logs (NetworkManager skips it if running)
  Serial.begin(OperatorConfig::BAUD_RATE);

  // Initialise the network stack — SoftAP + DNS + HTTP + WebSocket
  outputs.begin();     // everything off before anything else runs
  process.begin();
  process.startTask();
  bool networkOK = networkManager.begin(process);

  if (!networkOK) {
    // Fatal: networking failed — blink the built-in LED rapidly and halt.
    Serial.println(F("[FATAL] Network initialisation failed. System halted."));
    pinMode(LED_BUILTIN, OUTPUT);
    while (true) {
      digitalWrite(LED_BUILTIN, HIGH); delay(100);
      digitalWrite(LED_BUILTIN, LOW);  delay(100);
    }
  }

  // Initialise the Nextion touchscreen (non-fatal: runs fine with no display)
  nextion.begin();
  dashboard.begin();

  Serial.println(F("[MAIN] setup() complete — entering loop()"));
}

// ── loop() ───────────────────────────────────────────────────────────────────

void loop() {
  const uint32_t now = millis();

  // ── 1. Pump network services (must be called every iteration) ─────────────
  networkManager.handle();

  // ── 2. Pump the Nextion: restarts and operator touches ───────────────────
  nextion.handle();

  if (nextion.takeRestart()) {
    dashboard.begin();
  }

  uint16_t tx, ty;
  if (nextion.takeTouch(tx, ty)) {
    const HmiCommand c = dashboard.handleTouch(tx, ty);
    Serial.printf("[HMI ] Touch %u,%u -> %s\r\n", tx, ty, c.valid ? "button" : "no button");
    if (c.valid) {
      process.post(c.cmd, c.arg);
      _lastRenderMs = 0;   // redraw right after the command is applied
    }
  }

  // ── 3. HMI refresh (the batch FSM runs in its own task) ──────────────────
  if ((now - _lastRenderMs) >= RENDER_INTERVAL_MS) {
    _lastRenderMs = now;
    dashboard.render(process.snapshot(), networkManager.getClientCount(), now);
  }

  // ── 4. Client count change detection ─────────────────────────────────────
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

  // ── 5. Periodic heartbeat / status diagnostic ─────────────────────────────
  if ((now - _lastStatusPrintMs) >= STATUS_INTERVAL_MS) {
    _lastStatusPrintMs = now;

    Serial.println(F("[MAIN] ─── Heartbeat ────────────────────────────────────────"));
    Serial.printf ("[MAIN]  Uptime        : %lu ms\r\n",     (unsigned long)now);
    Serial.printf ("[MAIN]  Free Heap     : %u bytes\r\n",   ESP.getFreeHeap());
    Serial.printf ("[MAIN]  Min Free Heap : %u bytes\r\n",   ESP.getMinFreeHeap());
    Serial.printf ("[MAIN]  Active Clients: %d\r\n",         networkManager.getClientCount());
    Serial.printf ("[MAIN]  CPU Freq      : %u MHz\r\n",     getCpuFrequencyMhz());
    Serial.println(F("[MAIN] ─────────────────────────────────────────────────────────"));
  }
}
