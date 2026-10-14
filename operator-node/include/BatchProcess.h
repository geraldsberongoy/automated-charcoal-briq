#pragma once
// ============================================================================
// BatchProcess.h — Briquette Batch FSM (one machine action at a time)
//
// Flow (mockups/nextion/00_overview.png):
//   SETUP_PAPER    operator loads paper, picks water level (30-100 %, preset 100)
//   SETUP_CHARCOAL operator loads charcoal into the hopper, presses START
//   1 WATER        solenoid open until target volume
//   2 MIX          mixer motor on (timed)
//   3 OPEN_GATE    gate actuator retracts (full stroke)
//   4 POUR         mixer motor pulses (N bursts)
//   5 SHAKE        mixer-container vibrator on (timed)
//   6 CLOSE_GATE   gate actuator extends (full stroke)
//   7 LEVEL        compression-box vibrator on (timed)
//   8 PRESS        press actuator extends until target force, then holds
//   9 RELEASE      press actuator retracts (full stroke)
//   10 UNLOAD      all off; operator removes the briquette, presses DONE
//   STOPPED        STOP pressed: all off until RESET
//   RESETTING      press retracts, then gate closes, then back to SETUP_PAPER
//
// Every step drives exactly one output through Outputs::driveOnly().
//
// Sensors are SIMULATED (flow meter, S-type load cell) until their drivers
// exist — see the "simulated" comments in BatchProcess.cpp.
//
// Threading: tick() runs in its own FreeRTOS task (startTask(), every
// TICK_MS, above loop() priority) so display I/O can never delay a valve
// or actuator cut-off. Commands come from any task via post() (queue);
// readers get a copy via snapshot().
// ============================================================================

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "Outputs.h"

// Divide all durations (and multiply sim rates) for quick bench tests:
//   -DPROCESS_TIME_DIVISOR=10
#ifndef PROCESS_TIME_DIVISOR
  #define PROCESS_TIME_DIVISOR 1
#endif

namespace ProcessConfig {
  constexpr uint32_t DIV = PROCESS_TIME_DIVISOR;

  constexpr float    FULL_WATER_L      = 3.00f;   // volume at 100 % level
  constexpr uint8_t  WATER_MIN_PCT     = 30;
  constexpr uint8_t  WATER_MAX_PCT     = 100;
  constexpr uint8_t  WATER_STEP_PCT    = 10;

  constexpr uint32_t MIX_MS            = 180000UL / DIV;
  constexpr uint32_t GATE_STROKE_MS    = 5000UL   / DIV;
  constexpr uint8_t  POUR_PULSES       = 6;
  constexpr uint32_t POUR_ON_MS        = 2000UL   / DIV;
  constexpr uint32_t POUR_OFF_MS       = 3000UL   / DIV;
  constexpr uint32_t SHAKE_MS          = 15000UL  / DIV;
  constexpr uint32_t LEVEL_MS          = 30000UL  / DIV;
  constexpr float    PRESS_TARGET_N    = 2000.0f;
  constexpr uint32_t PRESS_HOLD_MS     = 3000UL   / DIV;
  constexpr uint32_t PRESS_STROKE_MS   = 18000UL  / DIV;  // full 40 cm travel
  constexpr uint32_t PRESS_TIMEOUT_MS  = 25000UL  / DIV;  // target never reached → stop

  // Simulated sensor rates
  constexpr float    SIM_FLOW_LPS      = 0.25f  * DIV;
  constexpr float    SIM_FORCE_NPS     = 150.0f * DIV;

  // FSM task
  constexpr uint32_t TICK_MS           = 10;
  constexpr uint32_t TASK_STACK_WORDS  = 4096;
  constexpr UBaseType_t TASK_PRIORITY  = 3;      // loop() runs at 1
  constexpr BaseType_t  TASK_CORE      = 1;
}

enum class Step : uint8_t {
  SETUP_PAPER, SETUP_CHARCOAL,
  WATER, MIX, OPEN_GATE, POUR, SHAKE, CLOSE_GATE, LEVEL, PRESS, RELEASE, UNLOAD,
  STOPPED, RESETTING
};

enum class Cmd : uint8_t { SET_WATER, NEXT, BACK, START, STOP, RESET, DONE };

struct BatchSnapshot {
  Step     step;
  uint8_t  runIndex;        // 1..10 for WATER..UNLOAD, 0 otherwise
  Step     stoppedDuring;   // valid in STOPPED
  uint8_t  waterPct;
  float    targetL;
  float    flowL;
  float    forceN;
  float    peakForceN;
  float    mixerA;
  uint32_t stepElapsedMs;
  uint32_t stepDurationMs;  // 0 when the step ends on a sensor, not a timer
  uint8_t  pulse;           // POUR: current burst (1-based)
  bool     pulseOn;
  bool     pressHolding;    // PRESS: target reached, holding
  uint8_t  resetPhase;      // RESETTING: 0 = press retract, 1 = gate close
  uint32_t batchMs;         // WATER start → UNLOAD
  Drive    outputs[static_cast<uint8_t>(Output::COUNT)];
  float    batteryV;
  float    solarA;
};

/**
 * @brief  Short upper-case name of a step ("WATER", "OPEN GATE", ...).
 */
const char *stepName(Step step);

class BatchProcess {
public:
  explicit BatchProcess(Outputs &outputs);

  /**
   * @brief  Create the command queue and switch every output off.
   */
  void begin();

  /**
   * @brief  Queue an operator command (thread-safe, non-blocking).
   *         arg is the water level % for SET_WATER, ignored otherwise.
   */
  void post(Cmd cmd, uint8_t arg = 0);

  /**
   * @brief  Run tick() every TICK_MS in a dedicated task. Call after begin().
   */
  void startTask();

  /**
   * @brief  Apply queued commands and advance the FSM (called by the task).
   */
  void tick();

  /**
   * @brief  Thread-safe copy of the current state.
   */
  BatchSnapshot snapshot() const;

private:
  struct Message { Cmd cmd; uint8_t arg; };

  Outputs      &_out;
  QueueHandle_t _queue;
  mutable portMUX_TYPE _mux;

  // Loop-task-only state
  Step     _step;
  Step     _stoppedDuring;
  uint8_t  _waterPct;
  float    _flowL;
  float    _forceN;
  float    _peakForceN;
  uint32_t _stepStartMs;
  uint32_t _batchStartMs;
  uint32_t _batchMs;
  uint32_t _lastTickMs;
  uint32_t _holdStartMs;    // PRESS: 0 until target reached
  uint8_t  _resetPhase;
  bool     _pulseOn;

  BatchSnapshot _snap;      // published copy, guarded by _mux

  void _apply(const Message &m);
  void _enter(Step step);
  void _advance(uint32_t now, float dtS);
  void _publish(uint32_t now);

  static void _taskEntry(void *self);
};
