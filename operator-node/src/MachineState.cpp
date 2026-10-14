// ============================================================================
// MachineState.cpp — Briquette Process FSM & Simulated Sensors
//
// See include/MachineState.h for public API documentation.
// ============================================================================

#include "MachineState.h"

namespace {
  // Idle sensor baseline at the start of a run
  constexpr float IDLE_TEMP_C  = 27.5f;
  constexpr float IDLE_RH_PCT  = 62.0f;

  float randUnit() {
    return static_cast<float>(esp_random()) / static_cast<float>(UINT32_MAX);
  }
}

// ── Constructor ──────────────────────────────────────────────────────────────

MachineState::MachineState()
  : _mux(portMUX_INITIALIZER_UNLOCKED)
{
  _s = MachineSnapshot{
    1,             // activeState
    true,          // safetyOk
    0.0f,          // flowL
    0.0f,          // forceN
    IDLE_TEMP_C,   // tempC
    IDLE_RH_PCT,   // rhPct
    0.0f,          // grinderA
    13.1f,         // batteryV
    1.8f           // solarA
  };
}

// ── Public: tick() ───────────────────────────────────────────────────────────

void MachineState::tick() {
  const float uptimeS = millis() / 1000.0f;

  portENTER_CRITICAL(&_mux);

  _s.batteryV = 12.9f + 0.3f * sinf(uptimeS / 60.0f);
  _s.solarA   = 1.5f + 0.5f * randUnit();

  if (!_s.safetyOk) {
    _s.grinderA = 0.0f;
    portEXIT_CRITICAL(&_mux);
    return;
  }

  switch (_s.activeState) {
    case 1:  // Soaking — water fills toward target
      _s.flowL    = min(ProcessConfig::TARGET_FLOW_L, _s.flowL + 0.25f);
      _s.grinderA = 0.0f;
      _s.forceN   = 0.0f;
      _s.tempC    = IDLE_TEMP_C;
      _s.rhPct    = IDLE_RH_PCT;
      break;
    case 2:  // Grinding — motor current wobbles around 4 A
      _s.flowL    = ProcessConfig::TARGET_FLOW_L;
      _s.grinderA = 3.8f + 0.6f * randUnit();
      break;
    case 3:  // Mixing
      _s.flowL    = ProcessConfig::TARGET_FLOW_L;
      _s.grinderA = 0.0f;
      break;
    case 4:  // Compaction — force ramps to target
      _s.forceN = min(ProcessConfig::TARGET_FORCE_N, _s.forceN + 150.0f);
      break;
    case 5:  // Drying — humidity falls, temperature rises
      if (_s.rhPct > 14.5f) {
        _s.rhPct  = max(14.5f, _s.rhPct - 3.5f);
        _s.tempC += 1.8f;
      }
      break;
    default:
      break;
  }

  portEXIT_CRITICAL(&_mux);
}

// ── Public: setState() / tripSafety() / resetFault() ─────────────────────────

void MachineState::setState(uint8_t state) {
  if (state < 1 || state > ProcessConfig::STATE_COUNT) return;

  portENTER_CRITICAL(&_mux);
  if (_s.safetyOk) {
    _s.activeState = state;
    switch (state) {
      case 1:
        _s.flowL = 0.0f; _s.forceN = 0.0f; _s.grinderA = 0.0f;
        _s.tempC = IDLE_TEMP_C; _s.rhPct = IDLE_RH_PCT;
        break;
      case 2: _s.flowL = ProcessConfig::TARGET_FLOW_L; _s.grinderA = 4.2f; break;
      case 3: _s.flowL = ProcessConfig::TARGET_FLOW_L; _s.grinderA = 0.0f; break;
      case 4: _s.forceN = 0.0f; break;
      case 5: _s.rhPct = IDLE_RH_PCT; _s.tempC = IDLE_TEMP_C; break;
    }
  }
  portEXIT_CRITICAL(&_mux);
}

void MachineState::tripSafety() {
  portENTER_CRITICAL(&_mux);
  _s.safetyOk = false;
  _s.forceN   = 850.0f;
  portEXIT_CRITICAL(&_mux);
}

void MachineState::resetFault() {
  portENTER_CRITICAL(&_mux);
  _s.safetyOk = true;
  portEXIT_CRITICAL(&_mux);
}

// ── Public: snapshot() ───────────────────────────────────────────────────────

MachineSnapshot MachineState::snapshot() const {
  portENTER_CRITICAL(&_mux);
  MachineSnapshot copy = _s;
  portEXIT_CRITICAL(&_mux);
  return copy;
}
