#pragma once
// ============================================================================
// MachineState.h — Briquette Process FSM & Sensor Telemetry Interface
//
// Single source of truth for the 5-stage process shared by the web dashboard
// (WebSocket) and the Nextion HMI:
//   1 Soaking → 2 Grinding → 3 Mixing → 4 Compaction → 5 Drying → 1 ...
//
// Sensor values are SIMULATED (mirrors dashboard/src/hooks/useStats.js mock)
// until the real flow meter, load cell, DS18B20 and DHT22 drivers exist.
//
// Thread safety: mutated from loop() (HMI touches), the async_tcp task
// (WebSocket commands) and read by the telemetry task — all access goes
// through a spinlock, and readers get a copy via snapshot().
// ============================================================================

#include <Arduino.h>

namespace ProcessConfig {
  constexpr float   TARGET_FLOW_L   = 3.00f;   // Soak water volume
  constexpr float   MAX_GRINDER_A   = 6.0f;    // Grinder current gauge full-scale
  constexpr float   TARGET_FORCE_N  = 2000.0f; // Compaction target
  constexpr float   TARGET_RH_PCT   = 15.0f;   // Drying complete threshold
  constexpr uint8_t STATE_COUNT     = 5;
}

struct MachineSnapshot {
  uint8_t activeState;   // 1..5
  bool    safetyOk;      // false = casing interlock open (fault)
  float   flowL;
  float   forceN;
  float   tempC;
  float   rhPct;
  float   grinderA;
  float   batteryV;
  float   solarA;
};

class MachineState {
public:
  MachineState();

  /**
   * @brief  Advance the simulated sensors by one step. Call once per second.
   */
  void tick();

  /**
   * @brief  Operator stage confirmation (1..5). Ignored while faulted.
   */
  void setState(uint8_t state);

  /**
   * @brief  Open the casing interlock (simulated safety breach).
   */
  void tripSafety();

  /**
   * @brief  Clear a safety fault (interlock closed again).
   */
  void resetFault();

  /**
   * @brief  Thread-safe copy of the current state.
   */
  MachineSnapshot snapshot() const;

private:
  mutable portMUX_TYPE _mux;
  MachineSnapshot      _s;
};
