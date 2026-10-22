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
// OUTPUTS_LIVE (build flag, default 1): the pin map below is the bench rig as
// wired and tested with actuator-test. Build with -DOUTPUTS_LIVE=0 to run the
// firmware against an ESP32 with the relays unplugged: the driver then only
// records the requested state and never touches GPIO.
// ============================================================================

#include <Arduino.h>

#ifndef OUTPUTS_LIVE
  #define OUTPUTS_LIVE 1
#endif

enum class Output : uint8_t {
  SOLENOID,      // water valve relay
  MIXER_MOTOR,   // 20V 5A mixer motor relay
  GATE_ACT,      // 15 cm gate actuator (dual relay)
  PRESS_ACT,     // 40 cm press actuator (dual relay)
  COUNT
};

enum class Drive : uint8_t { OFF, ON, EXTEND, RETRACT };

namespace OutputPins {
  // Relay boards are usually active-LOW (IN pulled low = relay on)
  constexpr bool RELAY_ACTIVE_LOW = true;

  // Same map as the bench rig (actuator-test, docs/guides/autobriq-perfboard-
  // layout.html): IN1..IN6 on the left header, D14 to D32 bottom to top.
  // GPIO14 pulses at boot, so IN1 (gate extend) may click once at power-up.
  // 13 = flow sensor, 16/17 = Nextion, 18/19 = HX711, so none are used here.
  constexpr int8_t GATE_A      = 14;   // IN1, extend
  constexpr int8_t GATE_B      = 27;   // IN2, retract
  constexpr int8_t PRESS_A     = 26;   // IN3, extend
  constexpr int8_t PRESS_B     = 25;   // IN4, retract
  constexpr int8_t MIXER_MOTOR = 33;   // IN5
  constexpr int8_t SOLENOID    = 32;   // IN6
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
