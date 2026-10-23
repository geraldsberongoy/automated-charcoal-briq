// process.js — Batch-flow metadata shared by the dashboard and the dev mock.
//
// Mirrors operator-node/include/BatchProcess.h (step names, timings, targets).
// Keep CONFIG in sync with ProcessConfig there.

export const CONFIG = {
  FULL_WATER_L:    3.0,     // volume at 100 % level
  WATER_LEVELS:    [100, 90, 80, 70, 60, 50, 40, 30],
  MIX_MS:          5000,
  GATE_STROKE_MS:  8000,    // demo timing, shorter than the full 15 cm stroke
  POUR_PULSES:     4,
  POUR_ON_MS:      1000,
  POUR_OFF_MS:     1000,
  PRESS_TARGET_N:  98,      // bench stop load (10 kg); final threshold comes from calibration (< 1,500 N actuator thrust)
  PRESS_HOLD_MS:   3000,
  RETRACT_FACTOR:  1.2,     // retracts run as long as the extension, x1.2 + margin
  RETRACT_MARGIN_MS: 1500,
  PRESS_STROKE_MS: 15000,   // dev mock only: assumed press travel time
};

// The 8 numbered steps (run_index 1..8), in order
export const RUN_STEPS = [
  { step: 'WATER',      label: 'Water' },
  { step: 'MIX',        label: 'Mix' },
  { step: 'OPEN GATE',  label: 'Open' },
  { step: 'POUR',       label: 'Pour' },
  { step: 'CLOSE GATE', label: 'Close' },
  { step: 'PRESS',      label: 'Press' },
  { step: 'RELEASE',    label: 'Release' },
  { step: 'UNLOAD',     label: 'Unload' },
];

export const STEP_TEXT = {
  'WATER':      { title: 'Dispensing water',          desc: 'Solenoid valve open. Filling the mixing container to the selected level.' },
  'MIX':        { title: 'Mixing',                    desc: 'Mixer motor running through its relay (20V, 5A).' },
  'OPEN GATE':  { title: 'Opening the gate',          desc: '15 cm actuator retracting (dual relay) to open the bottom of the mixing container.' },
  'POUR':       { title: 'Pouring into the press box', desc: 'Mixer motor spins in short bursts (1 s on, 1 s off) to push the mixture out through the gate.' },
  'CLOSE GATE': { title: 'Closing the gate',          desc: '15 cm actuator extending to close the mixing container.' },
  'PRESS':      { title: 'Compressing the briquette', desc: '40 cm actuator extending. Force read from the S-type load cell (HX711).' },
  'RELEASE':    { title: 'Releasing the press',       desc: '40 cm actuator retracting so the box can be removed.' },
};

// Output keys match the firmware telemetry "outputs" object
export const OUTPUTS = [
  { key: 'solenoid',       name: 'Water valve',    hw: 'Solenoid relay' },
  { key: 'mixer_motor',    name: 'Mixer motor',    hw: '20V 5A relay' },
  { key: 'gate_actuator',  name: 'Gate actuator',  hw: '15 cm, dual relay' },
  { key: 'press_actuator', name: 'Press actuator', hw: '40 cm, dual relay' },
];

export const RUNNING_STEPS = new Set(RUN_STEPS.slice(0, 7).map(s => s.step));

export function fmtMMSS(ms) {
  const s = Math.max(0, Math.floor((ms ?? 0) / 1000));
  return `${String(Math.floor(s / 60)).padStart(2, '0')}:${String(s % 60).padStart(2, '0')}`;
}

// Human label + colour variant for one output, same rules as the Nextion panel
export function outputDisplay(key, drive, proc) {
  const moving = drive === 'EXTEND' || drive === 'RETRACT';
  const gateOpen = proc?.step === 'POUR';

  switch (key) {
    case 'solenoid':
      return drive === 'ON' ? ['OPEN', 'accent'] : ['CLOSED', ''];
    case 'mixer_motor':
      if (proc?.step === 'POUR') return drive === 'ON' ? ['PULSE', 'info'] : ['REST', ''];
      return drive === 'ON' ? ['ON', 'accent'] : ['OFF', ''];
    case 'gate_actuator':
      if (moving) return [drive === 'EXTEND' ? 'EXTENDING' : 'RETRACTING', 'warn'];
      return [gateOpen ? 'OPEN' : 'CLOSED', ''];
    case 'press_actuator':
      if (moving) return [drive === 'EXTEND' ? 'EXTENDING' : 'RETRACTING', 'warn'];
      return proc?.press_holding ? ['HOLDING', 'warn'] : ['RETRACTED', ''];
    default:
      return drive === 'ON' ? ['ON', 'accent'] : ['OFF', ''];
  }
}
