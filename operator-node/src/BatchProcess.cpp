// ============================================================================
// BatchProcess.cpp — Briquette Batch FSM
//
// See include/BatchProcess.h for the flow and public API documentation.
// ============================================================================

#include "BatchProcess.h"

using namespace ProcessConfig;

namespace {
  constexpr uint8_t QUEUE_LEN = 8;

  bool isRunning(Step s) {
    return s >= Step::WATER && s <= Step::RELEASE;
  }

  float randUnit() {
    return static_cast<float>(esp_random()) / static_cast<float>(UINT32_MAX);
  }
}

const char *stepName(Step step) {
  switch (step) {
    case Step::SETUP_PAPER:    return "SETUP PAPER";
    case Step::SETUP_CHARCOAL: return "SETUP CHARCOAL";
    case Step::WATER:          return "WATER";
    case Step::MIX:            return "MIX";
    case Step::OPEN_GATE:      return "OPEN GATE";
    case Step::POUR:           return "POUR";
    case Step::CLOSE_GATE:     return "CLOSE GATE";
    case Step::PRESS:          return "PRESS";
    case Step::RELEASE:        return "RELEASE";
    case Step::UNLOAD:         return "UNLOAD";
    case Step::STOPPED:        return "STOPPED";
    case Step::RESETTING:      return "RESETTING";
  }
  return "?";
}

// ── Constructor ──────────────────────────────────────────────────────────────

BatchProcess::BatchProcess(Outputs &outputs)
  : _out(outputs),
    _queue(nullptr),
    _mux(portMUX_INITIALIZER_UNLOCKED),
    _step(Step::SETUP_PAPER),
    _stoppedDuring(Step::SETUP_PAPER),
    _waterPct(WATER_MAX_PCT),
    _flowL(0.0f),
    _forceN(0.0f),
    _peakForceN(0.0f),
    _stepStartMs(0),
    _batchStartMs(0),
    _batchMs(0),
    _lastTickMs(0),
    _holdStartMs(0),
    _resetPhase(0),
    _pressPosMs(0.0f),
    _gatePosMs(0.0f),
    _dwellMs(0),
    _pulseOn(false)
{
  memset(&_snap, 0, sizeof(_snap));
}

// ── Public: begin() / post() ─────────────────────────────────────────────────

void BatchProcess::begin() {
  _queue = xQueueCreate(QUEUE_LEN, sizeof(Message));
  _load.begin();
  _lastTickMs = millis();
  _enter(Step::SETUP_PAPER);
  _publish(_lastTickMs);
}

void BatchProcess::post(Cmd cmd, uint8_t arg) {
  if (!_queue) return;
  const Message m{cmd, arg};
  xQueueSend(_queue, &m, 0);   // drop if full — operator can press again
}

// ── Public: startTask() ──────────────────────────────────────────────────────

void BatchProcess::startTask() {
  xTaskCreatePinnedToCore(_taskEntry, "batch_fsm", TASK_STACK_WORDS, this,
                          TASK_PRIORITY, nullptr, TASK_CORE);
}

void BatchProcess::_taskEntry(void *self) {
  BatchProcess *p = static_cast<BatchProcess*>(self);
  TickType_t last = xTaskGetTickCount();
  for (;;) {
    p->tick();
    vTaskDelayUntil(&last, pdMS_TO_TICKS(TICK_MS));
  }
}

// ── Public: tick() ───────────────────────────────────────────────────────────

void BatchProcess::tick() {
  const uint32_t now = millis();
  const float dtS = (now - _lastTickMs) / 1000.0f;
  _lastTickMs = now;

  _trackPosition(dtS * 1000.0f);   // drives set last tick moved the actuators
  _load.poll();
  _forceN = _load.ready() ? max(0.0f, _load.newtons()) : 0.0f;

  Message m;
  while (_queue && xQueueReceive(_queue, &m, 0) == pdTRUE) {
    _apply(m);
  }

  _advance(now, dtS);
  _publish(now);
}

// ── Public: snapshot() ───────────────────────────────────────────────────────

BatchSnapshot BatchProcess::snapshot() const {
  portENTER_CRITICAL(&_mux);
  BatchSnapshot copy = _snap;
  portEXIT_CRITICAL(&_mux);
  return copy;
}

// ── Private: _apply() — operator commands ────────────────────────────────────

void BatchProcess::_apply(const Message &m) {
  switch (m.cmd) {
    case Cmd::SET_WATER:
      if (_step == Step::SETUP_PAPER) {
        const uint8_t pct = constrain(m.arg, WATER_MIN_PCT, WATER_MAX_PCT);
        _waterPct = (pct / WATER_STEP_PCT) * WATER_STEP_PCT;
      }
      break;
    case Cmd::NEXT:
      if (_step == Step::SETUP_PAPER) _enter(Step::SETUP_CHARCOAL);
      break;
    case Cmd::BACK:
      if (_step == Step::SETUP_CHARCOAL) _enter(Step::SETUP_PAPER);
      break;
    case Cmd::START:
      if (_step == Step::SETUP_CHARCOAL) {
        _flowL = 0.0f;
        _forceN = 0.0f;
        _peakForceN = 0.0f;
        _batchStartMs = millis();
        _batchMs = 0;
        _enter(Step::WATER);
      }
      break;
    case Cmd::STOP:
      if (isRunning(_step)) {
        _stoppedDuring = _step;
        _enter(Step::STOPPED);
      }
      break;
    case Cmd::RESET:
      if (_step == Step::STOPPED) _enter(Step::RESETTING);
      break;
    case Cmd::DONE:
      if (_step == Step::UNLOAD) {
        _waterPct = WATER_MAX_PCT;   // back to the preset
        _enter(Step::SETUP_PAPER);
      }
      break;
  }
}

// ── Private: _enter() — one output per step ──────────────────────────────────

void BatchProcess::_enter(Step step) {
  _step        = step;
  _stepStartMs = millis();
  _holdStartMs = 0;

  Serial.printf("[PROC] -> %s\r\n", stepName(step));

  switch (step) {
    case Step::WATER:      _out.driveOnly(Output::SOLENOID,    Drive::ON);      break;
    case Step::MIX:        _out.driveOnly(Output::MIXER_MOTOR, Drive::ON);      break;
    case Step::OPEN_GATE:  _out.driveOnly(Output::GATE_ACT,    Drive::RETRACT); break;
    case Step::POUR:
      _pulseOn = true;
      _out.driveOnly(Output::MIXER_MOTOR, Drive::ON);
      break;
    case Step::CLOSE_GATE:
      _dwellMs = _retractMs(_gatePosMs);
      _out.driveOnly(Output::GATE_ACT, Drive::EXTEND);
      break;
    case Step::PRESS:      _out.driveOnly(Output::PRESS_ACT,   Drive::EXTEND);  break;
    case Step::RELEASE:
      _dwellMs = _retractMs(_pressPosMs);
      _out.driveOnly(Output::PRESS_ACT, Drive::RETRACT);
      break;
    case Step::RESETTING:
      _resetPhase = 0;
      _dwellMs    = _retractMs(_pressPosMs);
      _out.driveOnly(Output::PRESS_ACT, Drive::RETRACT);
      break;
    case Step::UNLOAD:
      _batchMs = millis() - _batchStartMs;
      _out.allOff();
      break;
    default:                // SETUP_*, STOPPED
      _out.allOff();
      break;
  }
}

// ── Private: _advance() — timers and sensor thresholds ───────────────────────

void BatchProcess::_advance(uint32_t now, float dtS) {
  // a command applied this tick may have restarted the step after `now` was read
  const uint32_t elapsed = now > _stepStartMs ? now - _stepStartMs : 0;

  switch (_step) {
    case Step::WATER:
      _flowL += SIM_FLOW_LPS * dtS;              // estimated from valve time (flow sensor off)
      if (_flowL >= FULL_WATER_L * _waterPct / 100.0f) _enter(Step::MIX);
      break;

    case Step::MIX:
      if (elapsed >= MIX_MS) _enter(Step::OPEN_GATE);
      break;

    case Step::OPEN_GATE:
      if (elapsed >= GATE_STROKE_MS) _enter(Step::POUR);
      break;

    case Step::POUR: {
      const uint32_t cycle = POUR_ON_MS + POUR_OFF_MS;
      if (elapsed >= cycle * POUR_PULSES) {
        _enter(Step::CLOSE_GATE);
        break;
      }
      const bool on = (elapsed % cycle) < POUR_ON_MS;
      if (on != _pulseOn) {
        _pulseOn = on;
        if (on) _out.driveOnly(Output::MIXER_MOTOR, Drive::ON);
        else    _out.allOff();
      }
      break;
    }

    case Step::CLOSE_GATE:
      if (elapsed >= _dwellMs) _enter(Step::PRESS);
      break;

    case Step::PRESS:
      _peakForceN = max(_peakForceN, _forceN);
      if (_holdStartMs == 0) {
        if (_load.ready() && _forceN >= PRESS_TARGET_N) {
          _holdStartMs = now;
          _out.allOff();                          // actuator holds position unpowered
        } else if (elapsed >= PRESS_TIMEOUT_MS) {
          // Not an emergency: the cycle goes on and the press retracts home
          Serial.println(F("[PROC] Press target not reached in time, retracting"));
          _enter(Step::RELEASE);
        }
      } else if (now - _holdStartMs >= PRESS_HOLD_MS) {
        _enter(Step::RELEASE);
      }
      break;

    case Step::RELEASE:
      if (elapsed >= _dwellMs) _enter(Step::UNLOAD);
      break;

    case Step::RESETTING:
      if (_resetPhase == 0 && elapsed >= _dwellMs) {
        _resetPhase = 1;
        _stepStartMs = now;
        _dwellMs = _retractMs(_gatePosMs);
        _out.driveOnly(Output::GATE_ACT, Drive::EXTEND);
      } else if (_resetPhase == 1 && elapsed >= _dwellMs) {
        _waterPct = WATER_MAX_PCT;
        _enter(Step::SETUP_PAPER);
      }
      break;

    default:
      break;
  }
}

// ── Private: actuator position estimate ──────────────────────────────────────

void BatchProcess::_trackPosition(float dtMs) {
  const Drive p = _out.state(Output::PRESS_ACT);
  const Drive g = _out.state(Output::GATE_ACT);
  if (p == Drive::EXTEND)       _pressPosMs += dtMs;
  else if (p == Drive::RETRACT) _pressPosMs = max(0.0f, _pressPosMs - dtMs);
  if (g == Drive::RETRACT)      _gatePosMs += dtMs;      // retract = open
  else if (g == Drive::EXTEND)  _gatePosMs = max(0.0f, _gatePosMs - dtMs);
}

uint32_t BatchProcess::_retractMs(float posMs) {
  return static_cast<uint32_t>(posMs * RETRACT_FACTOR) + RETRACT_MARGIN_MS;
}

// ── Private: _publish() ──────────────────────────────────────────────────────

void BatchProcess::_publish(uint32_t now) {
  BatchSnapshot s;
  s.step          = _step;
  s.runIndex      = (_step >= Step::WATER && _step <= Step::UNLOAD)
                      ? static_cast<uint8_t>(_step) - static_cast<uint8_t>(Step::WATER) + 1 : 0;
  s.stoppedDuring = _stoppedDuring;
  s.waterPct      = _waterPct;
  s.targetL       = FULL_WATER_L * _waterPct / 100.0f;
  s.flowL         = _flowL;
  s.forceN        = _forceN;
  s.peakForceN    = _peakForceN;
  s.mixerA        = _out.state(Output::MIXER_MOTOR) == Drive::ON ? 4.6f + 0.4f * randUnit() : 0.0f;
  s.stepElapsedMs = now > _stepStartMs ? now - _stepStartMs : 0;
  s.pressHolding  = _step == Step::PRESS && _holdStartMs != 0;
  s.resetPhase    = _resetPhase;
  s.pulseOn       = _pulseOn;

  const uint32_t cycle = POUR_ON_MS + POUR_OFF_MS;
  s.pulse = _step == Step::POUR ? min<uint32_t>(POUR_PULSES, s.stepElapsedMs / cycle + 1) : 0;

  switch (_step) {
    case Step::MIX:        s.stepDurationMs = MIX_MS; break;
    case Step::OPEN_GATE:
    case Step::CLOSE_GATE: s.stepDurationMs = _dwellMs; break;
    case Step::POUR:       s.stepDurationMs = cycle * POUR_PULSES; break;
    case Step::RELEASE:
    case Step::RESETTING:  s.stepDurationMs = _dwellMs; break;
    default:               s.stepDurationMs = 0; break;
  }

  s.batchMs = (_step == Step::UNLOAD) ? _batchMs
            : isRunning(_step)        ? now - _batchStartMs : 0;

  for (uint8_t i = 0; i < static_cast<uint8_t>(Output::COUNT); ++i) {
    s.outputs[i] = _out.state(static_cast<Output>(i));
  }

  portENTER_CRITICAL(&_mux);
  _snap = s;
  portEXIT_CRITICAL(&_mux);
}
