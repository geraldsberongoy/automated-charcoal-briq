#pragma once
// ============================================================================
// Outputs.h — Relay / Actuator Output Driver (one output at a time)
//
// Owns every machine output and enforces the core safety rule of the batch
// flow: AT MOST ONE output is energised at any moment. driveOnly() switches
// everything else off before switching the requested output on.
//
// Dual-relay linear actuators (gate 15 cm, press 40 cm) use two relays:
//   EXTEND  = relay A on,  relay B off
//   RETRACT = relay A off, relay B on
//   OFF     = both off      (A and B are never on together)
//
// OUTPUTS_LIVE (build flag, default 0): while 0 the driver only records the
// requested state and never touches GPIO, so the firmware can run against a
// bench ESP32 with relays unplugged or wired differently. Set
// -DOUTPUTS_LIVE=1 only after the pin map below matches the real wiring.
// ============================================================================

#include <Arduino.h>

#ifndef OUTPUTS_LIVE
  #define OUTPUTS_LIVE 0
#endif

enum class Output : uint8_t {
  SOLENOID,      // water valve relay
  MIXER_MOTOR,   // 20V 5A mixer motor relay
  GATE_ACT,      // 15 cm gate actuator (dual relay)
  MIXER_ERM,     // vibration motor on the mixing container
  BOX_ERM,       // vibration motor on the compression box
  PRESS_ACT,     // 40 cm press actuator (dual relay)
  COUNT
};

enum class Drive : uint8_t { OFF, ON, EXTEND, RETRACT };

namespace OutputPins {
  // Relay boards are usually active-LOW (IN pulled low = relay on)
  constexpr bool RELAY_ACTIVE_LOW = true;

  // Avoids UART2 (16/17), strapping pins (0/2/12/15), flash (6-11),
  // input-only pins (34-39) and GPIO14 (PWM glitch at boot).
  constexpr int8_t SOLENOID    = 25;
  constexpr int8_t MIXER_MOTOR = 26;
  constexpr int8_t GATE_A      = 27;   // extend
  constexpr int8_t GATE_B      = 13;   // retract
  constexpr int8_t MIXER_ERM   = 32;
  constexpr int8_t BOX_ERM     = 33;
  constexpr int8_t PRESS_A     = 18;   // extend
  constexpr int8_t PRESS_B     = 19;   // retract
}

class Outputs {
public:
  Outputs();

  /**
   * @brief  Configure pins (if OUTPUTS_LIVE) and switch everything off.
   */
  void begin();

  /**
   * @brief  Switch every output off, then drive only `out`.
   *         ON is for single relays; EXTEND/RETRACT for actuators.
   */
  void driveOnly(Output out, Drive drive);

  /**
   * @brief  Switch every output off.
   */
  void allOff();

  Drive state(Output out) const { return _state[static_cast<uint8_t>(out)]; }

private:
  Drive _state[static_cast<uint8_t>(Output::COUNT)];

  void _write(int8_t pin, bool on);
  void _apply(Output out, Drive drive);
};
