// mockProcess.js — Dev-mode simulator of the firmware batch FSM.
//
// Mirrors operator-node/src/BatchProcess.cpp so `npm run dev` shows the real
// flow without an ESP32. Timers run MOCK_SPEED× faster than the machine.

import { CONFIG } from '../process';

const MOCK_SPEED = 10;
const C = Object.fromEntries(Object.entries(CONFIG).map(([k, v]) =>
  [k, k.endsWith('_MS') ? v / MOCK_SPEED : v]));

const SIM_FLOW_LPS  = 0.30  * MOCK_SPEED;   // valve time, same as the firmware
const SIM_FORCE_NPS = 15.0  * MOCK_SPEED;   // stands in for the load cell in dev mode

const RUNNING = ['WATER', 'MIX', 'OPEN GATE', 'POUR', 'CLOSE GATE', 'PRESS', 'RELEASE'];
const ORDER   = [...RUNNING, 'UNLOAD'];

const ALL_OFF = {
  solenoid: 'OFF', mixer_motor: 'OFF', gate_actuator: 'OFF',
  press_actuator: 'OFF',
};

// The one output each step drives (everything else off)
const STEP_OUTPUT = {
  'WATER':      ['solenoid', 'ON'],
  'MIX':        ['mixer_motor', 'ON'],
  'OPEN GATE':  ['gate_actuator', 'RETRACT'],
  'POUR':       ['mixer_motor', 'ON'],
  'CLOSE GATE': ['gate_actuator', 'EXTEND'],
  'PRESS':      ['press_actuator', 'EXTEND'],
  'RELEASE':    ['press_actuator', 'RETRACT'],
  'RESETTING':  ['press_actuator', 'RETRACT'],
};

export function createMock(now) {
  const s = {
    step: 'SETUP PAPER', stoppedDuring: 'SETUP PAPER', waterPct: 100,
    flowL: 0, forceN: 0, peakForceN: 0,
    stepStart: now, batchStart: 0, batchMs: 0, holdStart: 0, resetPhase: 0,
    outputs: { ...ALL_OFF },
  };
  return s;
}

function enter(s, step, now) {
  s.step = step;
  s.stepStart = now;
  s.holdStart = 0;
  s.outputs = { ...ALL_OFF };
  const out = STEP_OUTPUT[step];
  if (out) s.outputs[out[0]] = out[1];
  if (step === 'RESETTING') s.resetPhase = 0;
  if (step === 'UNLOAD') s.batchMs = now - s.batchStart;
}

export function applyCommand(s, action, payload, now) {
  switch (action) {
    case 'SET_WATER':
      if (s.step === 'SETUP PAPER') {
        const pct = Math.min(100, Math.max(30, Number(payload?.pct) || 100));
        s.waterPct = Math.floor(pct / 10) * 10;
      }
      break;
    case 'NEXT':  if (s.step === 'SETUP PAPER') enter(s, 'SETUP CHARCOAL', now); break;
    case 'BACK':  if (s.step === 'SETUP CHARCOAL') enter(s, 'SETUP PAPER', now); break;
    case 'START':
      if (s.step === 'SETUP CHARCOAL') {
        s.flowL = 0; s.forceN = 0; s.peakForceN = 0; s.batchStart = now; s.batchMs = 0;
        enter(s, 'WATER', now);
      }
      break;
    case 'STOP':
    case 'TRIP_SAFETY':
      if (RUNNING.includes(s.step)) { s.stoppedDuring = s.step; enter(s, 'STOPPED', now); }
      break;
    case 'RESET': if (s.step === 'STOPPED') enter(s, 'RESETTING', now); break;
    case 'DONE':
      if (s.step === 'UNLOAD') { s.waterPct = 100; enter(s, 'SETUP PAPER', now); }
      break;
    default: break;
  }
}

export function advance(s, now, dtS) {
  const elapsed = now - s.stepStart;
  const cycle = C.POUR_ON_MS + C.POUR_OFF_MS;

  switch (s.step) {
    case 'WATER':
      s.flowL += SIM_FLOW_LPS * dtS;
      if (s.flowL >= C.FULL_WATER_L * s.waterPct / 100) enter(s, 'MIX', now);
      break;
    case 'MIX':        if (elapsed >= C.MIX_MS) enter(s, 'OPEN GATE', now); break;
    case 'OPEN GATE':  if (elapsed >= C.GATE_STROKE_MS) enter(s, 'POUR', now); break;
    case 'POUR':
      if (elapsed >= cycle * C.POUR_PULSES) enter(s, 'CLOSE GATE', now);
      else s.outputs.mixer_motor = (elapsed % cycle) < C.POUR_ON_MS ? 'ON' : 'OFF';
      break;
    case 'CLOSE GATE': if (elapsed >= C.GATE_STROKE_MS) enter(s, 'PRESS', now); break;
    case 'PRESS':
      if (!s.holdStart) {
        s.forceN += SIM_FORCE_NPS * dtS;
        if (s.forceN >= C.PRESS_TARGET_N) {
          s.forceN = C.PRESS_TARGET_N + 10 * Math.random();
          s.holdStart = now;
          s.outputs = { ...ALL_OFF };
        }
        s.peakForceN = Math.max(s.peakForceN, s.forceN);
      } else if (now - s.holdStart >= C.PRESS_HOLD_MS) {
        enter(s, 'RELEASE', now);
      }
      break;
    case 'RELEASE':
      s.forceN = Math.max(0, s.forceN - SIM_FORCE_NPS * 4 * dtS);
      if (elapsed >= C.PRESS_STROKE_MS) enter(s, 'UNLOAD', now);
      break;
    case 'RESETTING':
      s.forceN = Math.max(0, s.forceN - SIM_FORCE_NPS * 4 * dtS);
      if (s.resetPhase === 0 && elapsed >= C.RESET_PHASE_MS) {
        s.resetPhase = 1;
        s.stepStart = now;
        s.outputs = { ...ALL_OFF, gate_actuator: 'EXTEND' };
      } else if (s.resetPhase === 1 && elapsed >= C.RESET_PHASE_MS) {
        s.waterPct = 100;
        enter(s, 'SETUP PAPER', now);
      }
      break;
    default: break;
  }
}

function durationOf(s) {
  switch (s.step) {
    case 'MIX':        return C.MIX_MS;
    case 'OPEN GATE':
    case 'CLOSE GATE': return C.GATE_STROKE_MS;
    case 'POUR':       return (C.POUR_ON_MS + C.POUR_OFF_MS) * C.POUR_PULSES;
    case 'RELEASE':    return C.PRESS_STROKE_MS;
    case 'RESETTING':  return C.RESET_PHASE_MS;
    default:           return 0;
  }
}

// Same shape as the firmware's "process" telemetry object.
// Durations are scaled back up so the UI shows real machine times.
export function toProcess(s, now) {
  const idx = ORDER.indexOf(s.step);
  const elapsed = now - s.stepStart;
  const cycle = C.POUR_ON_MS + C.POUR_OFF_MS;
  return {
    step: s.step,
    run_index: idx >= 0 ? idx + 1 : 0,
    stopped_during: s.stoppedDuring,
    water_pct: s.waterPct,
    target_l: +(CONFIG.FULL_WATER_L * s.waterPct / 100).toFixed(2),
    flow_l: +s.flowL.toFixed(2),
    force_n: Math.round(s.forceN),
    peak_force_n: Math.round(s.peakForceN),
    mixer_a: s.outputs.mixer_motor === 'ON' ? +(4.6 + 0.4 * Math.random()).toFixed(1) : 0,
    step_elapsed_ms: elapsed * MOCK_SPEED,
    step_duration_ms: durationOf(s) * MOCK_SPEED,
    pulse: s.step === 'POUR' ? Math.min(C.POUR_PULSES, Math.floor(elapsed / cycle) + 1) : 0,
    press_holding: s.step === 'PRESS' && s.holdStart !== 0,
    reset_phase: s.resetPhase,
    batch_ms: (s.step === 'UNLOAD' ? s.batchMs
             : RUNNING.includes(s.step) ? now - s.batchStart : 0) * MOCK_SPEED,
  };
}
