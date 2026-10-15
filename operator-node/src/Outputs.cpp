// ============================================================================
// Outputs.cpp — Relay / Actuator Output Driver
//
// See include/Outputs.h for public API documentation.
// ============================================================================

#include "Outputs.h"

#if OUTPUTS_LIVE
namespace {
  constexpr int8_t ALL_PINS[] = {
    OutputPins::SOLENOID, OutputPins::MIXER_MOTOR,
    OutputPins::GATE_A, OutputPins::GATE_B,
    OutputPins::MIXER_ERM, OutputPins::BOX_ERM,
    OutputPins::PRESS_A, OutputPins::PRESS_B,
  };
}
#endif

// ── Constructor ──────────────────────────────────────────────────────────────

Outputs::Outputs() {
  for (auto &s : _state) s = Drive::OFF;
}

// ── Public: begin() ──────────────────────────────────────────────────────────

void Outputs::begin() {
#if OUTPUTS_LIVE
  for (int8_t pin : ALL_PINS) {
    _write(pin, false);           // set the off level before enabling output
    pinMode(pin, OUTPUT);
  }
  Serial.println(F("[OUT ] Relay outputs LIVE"));
#else
  Serial.println(F("[OUT ] Relay outputs SIMULATED (build with -DOUTPUTS_LIVE=1 to drive GPIO)"));
#endif
  allOff();
}

// ── Public: driveOnly() / allOff() ───────────────────────────────────────────

void Outputs::driveOnly(Output out, Drive drive) {
  // Everything else off first — never two outputs energised at once
  for (uint8_t i = 0; i < static_cast<uint8_t>(Output::COUNT); ++i) {
    if (i != static_cast<uint8_t>(out)) _apply(static_cast<Output>(i), Drive::OFF);
  }
  _apply(out, drive);
}

void Outputs::allOff() {
  for (uint8_t i = 0; i < static_cast<uint8_t>(Output::COUNT); ++i) {
    _apply(static_cast<Output>(i), Drive::OFF);
  }
}

// ── Private: _apply() / _write() ─────────────────────────────────────────────

void Outputs::_apply(Output out, Drive drive) {
  _state[static_cast<uint8_t>(out)] = drive;

  switch (out) {
    case Output::SOLENOID:    _write(OutputPins::SOLENOID,    drive == Drive::ON); break;
    case Output::MIXER_MOTOR: _write(OutputPins::MIXER_MOTOR, drive == Drive::ON); break;
    case Output::MIXER_ERM:   _write(OutputPins::MIXER_ERM,   drive == Drive::ON); break;
    case Output::BOX_ERM:     _write(OutputPins::BOX_ERM,     drive == Drive::ON); break;

    case Output::GATE_ACT:
      // Release the opposing relay before engaging, so A and B never overlap
      if (drive == Drive::EXTEND) {
        _write(OutputPins::GATE_B, false); _write(OutputPins::GATE_A, true);
      } else if (drive == Drive::RETRACT) {
        _write(OutputPins::GATE_A, false); _write(OutputPins::GATE_B, true);
      } else {
        _write(OutputPins::GATE_A, false); _write(OutputPins::GATE_B, false);
      }
      break;

    case Output::PRESS_ACT:
      if (drive == Drive::EXTEND) {
        _write(OutputPins::PRESS_B, false); _write(OutputPins::PRESS_A, true);
      } else if (drive == Drive::RETRACT) {
        _write(OutputPins::PRESS_A, false); _write(OutputPins::PRESS_B, true);
      } else {
        _write(OutputPins::PRESS_A, false); _write(OutputPins::PRESS_B, false);
      }
      break;

    default:
      break;
  }
}

void Outputs::_write(int8_t pin, bool on) {
#if OUTPUTS_LIVE
  const bool level = OutputPins::RELAY_ACTIVE_LOW ? !on : on;
  digitalWrite(pin, level ? HIGH : LOW);
#else
  (void)pin;
  (void)on;
#endif
}
