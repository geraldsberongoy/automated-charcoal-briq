// BatchWorkspace.jsx — Operator workspace for the one-action-at-a-time batch flow.
// Same steps, buttons and output rules as the Nextion screens on the machine.
import KpiCard  from './KpiCard';
import Badge    from './Badge';
import EventLog from './EventLog';
import {
  FiPlay, FiSquare, FiRotateCcw, FiArrowLeft, FiArrowRight, FiCheck,
  FiDroplet, FiActivity, FiZap, FiClock, FiAlertTriangle,
} from 'react-icons/fi';
import {
  CONFIG, RUN_STEPS, STEP_TEXT, OUTPUTS, RUNNING_STEPS, fmtMMSS, outputDisplay,
} from '../process';

// ── Progress gauge for the running step (label, value, 0..1, colour) ─────────
function gaugeFor(p) {
  const timed = p.step_duration_ms > 0 ? p.step_elapsed_ms / p.step_duration_ms : 0;
  const left  = fmtMMSS(p.step_duration_ms - p.step_elapsed_ms);

  switch (p.step) {
    case 'WATER':
      return [`Water (target ${p.water_pct}% = ${p.target_l.toFixed(2)} L)`,
              `${p.flow_l.toFixed(2)} L`, p.target_l ? p.flow_l / p.target_l : 0];
    case 'MIX':
      return ['Mixing time left', `${left}  ·  ${p.mixer_a.toFixed(1)} A`, timed];
    case 'SHAKE':
    case 'LEVEL':
      return ['Time left', left, timed];
    case 'OPEN GATE':
    case 'CLOSE GATE':
    case 'RELEASE':
      return ['Actuator stroke', `${(p.step_elapsed_ms / 1000).toFixed(1)} s / ~${Math.ceil(p.step_duration_ms / 1000)} s`, timed];
    case 'POUR':
      return [`Bursts (${CONFIG.POUR_ON_MS / 1000} s on / ${CONFIG.POUR_OFF_MS / 1000} s off)`,
              `${p.pulse} / ${CONFIG.POUR_PULSES}`, timed];
    case 'PRESS':
      return [`Load cell (target ${CONFIG.PRESS_TARGET_N} N)`,
              p.press_holding ? `${p.force_n} N · HOLDING` : `${p.force_n} N`,
              p.force_n / CONFIG.PRESS_TARGET_N, 'warning'];
    default:
      return null;
  }
}

function Gauge({ label, value, fraction, variant = '' }) {
  return (
    <div className="gauge-wrap">
      <div className="gauge-label"><span>{label}</span><span>{value}</span></div>
      <div className="gauge-track">
        <div className={`gauge-fill ${variant}`}
             style={{ width: `${Math.min(100, Math.max(0, fraction * 100))}%` }} />
      </div>
    </div>
  );
}

// ── Operator panel: what to do right now ─────────────────────────────────────
function OperatorPanel({ p, sendCommand }) {
  if (p.step === 'SETUP PAPER') {
    return (
      <>
        <div className="prompt-title">Setup 1 of 2: Load the paper</div>
        <p className="prompt-desc">
          Put the shredded paper into the mixing container, then choose how much water to add.
          The level can be changed until you press Next.
        </p>
        <div className="water-picker">
          <div className="vial" aria-hidden="true">
            <div className="vial__fill" style={{ height: `${p.water_pct}%` }} />
          </div>
          <div>
            <div className="water-picker__value">{p.water_pct}%</div>
            <div className="water-picker__sub">= {p.target_l.toFixed(2)} L of water</div>
            <div className="level-grid">
              {CONFIG.WATER_LEVELS.map(v => (
                <button key={v}
                        className={`level-btn ${v === p.water_pct ? 'active' : ''}`}
                        onClick={() => sendCommand('SET_WATER', { pct: v })}>
                  {v}%
                </button>
              ))}
            </div>
          </div>
        </div>
        <div className="btn-row">
          <button className="action-btn" onClick={() => sendCommand('NEXT')}>
            Paper loaded · Next <FiArrowRight />
          </button>
        </div>
      </>
    );
  }

  if (p.step === 'SETUP CHARCOAL') {
    return (
      <>
        <div className="prompt-title">Setup 2 of 2: Load the charcoal</div>
        <p className="prompt-desc">
          Pour the crushed charcoal into the hopper. When everything is loaded, press Start.
          The process then runs on its own, one machine action at a time.
        </p>
        <div className="btn-row" style={{ marginBottom: '1.25rem' }}>
          <Badge variant="green">✓ Paper loaded</Badge>
          <Badge variant="green">✓ Water {p.water_pct}% ({p.target_l.toFixed(2)} L)</Badge>
        </div>
        <div className="btn-row">
          <button className="action-btn ghost" onClick={() => sendCommand('BACK')}>
            <FiArrowLeft /> Back
          </button>
          <button className="action-btn" onClick={() => sendCommand('START')}>
            <FiPlay /> Start process
          </button>
        </div>
      </>
    );
  }

  if (RUNNING_STEPS.has(p.step)) {
    const text  = STEP_TEXT[p.step];
    const gauge = gaugeFor(p);
    return (
      <>
        <div className="prompt-title">Step {p.run_index}: {text.title}</div>
        <p className="prompt-desc">{text.desc}</p>
        {gauge && <Gauge label={gauge[0]} value={gauge[1]} fraction={gauge[2]} variant={gauge[3]} />}
        {p.step === 'PRESS' && (
          <p className="prompt-desc" style={{ color: 'var(--clr-warn)', fontWeight: 600 }}>
            <FiAlertTriangle /> Keep hands clear of the press.
          </p>
        )}
        <div className="btn-row">
          <button className="action-btn danger" onClick={() => sendCommand('STOP')}>
            <FiSquare /> Stop
          </button>
        </div>
      </>
    );
  }

  if (p.step === 'UNLOAD') {
    return (
      <>
        <div className="prompt-title">Step 10: Briquette ready</div>
        <p className="prompt-desc">
          All motors are off. Remove the press box and put the briquette into your drying container.
        </p>
        <div className="unload-stats">
          <div><span>Water</span><b>{p.target_l.toFixed(2)} L</b></div>
          <div><span>Mix</span><b>{fmtMMSS(CONFIG.MIX_MS)}</b></div>
          <div><span>Peak force</span><b>{p.peak_force_n} N</b></div>
          <div><span>Total</span><b>{fmtMMSS(p.batch_ms)}</b></div>
        </div>
        <div className="btn-row">
          <button className="action-btn" onClick={() => sendCommand('DONE')}>
            <FiCheck /> Done · New batch
          </button>
        </div>
      </>
    );
  }

  if (p.step === 'RESETTING') {
    const frac = p.step_duration_ms ? p.step_elapsed_ms / p.step_duration_ms : 0;
    return (
      <>
        <div className="prompt-title">Resetting</div>
        <p className="prompt-desc">Returning the press and gate to their start positions. Keep clear.</p>
        <Gauge label="Homing"
               value={p.reset_phase === 0 ? '1 of 2: retracting press' : '2 of 2: closing gate'}
               fraction={frac} variant="warning" />
      </>
    );
  }

  // STOPPED — the full-screen overlay in App.jsx carries the reset button
  return (
    <>
      <div className="prompt-title" style={{ color: 'var(--clr-danger)' }}>Process stopped</div>
      <p className="prompt-desc">Stopped during {p.stopped_during}. All relays are off.</p>
      <button className="action-btn danger" onClick={() => sendCommand('RESET')}>
        <FiRotateCcw /> Reset &amp; retract
      </button>
    </>
  );
}

// ── Workspace ────────────────────────────────────────────────────────────────
export default function BatchWorkspace({ stats, loading, sendCommand, logEntries }) {
  const p = stats?.process;
  const outputs = stats?.outputs ?? {};
  const active = p?.run_index ?? 0;
  const pressActive = p?.step === 'PRESS' || p?.step === 'RELEASE';

  return (
    <>
      <p className="section-label">Batch Progress · one machine action at a time</p>
      <div className="wizard wizard--10">
        {RUN_STEPS.map((s, idx) => {
          const n = idx + 1;
          const isActive = n === active;
          const isCompleted = active > n;
          return (
            <div key={s.step} className={`wizard-step ${isActive ? 'active' : ''} ${isCompleted ? 'completed' : ''}`}>
              <div className="wizard-step__circle">{isCompleted ? '✓' : n}</div>
              <div className="wizard-step__info">
                <span className="wizard-step__name">{s.label}</span>
              </div>
            </div>
          );
        })}
      </div>

      <p className="section-label">Operator Panel</p>
      <div className={`prompt-card ${p ? 'active-step' : ''}`}>
        {p ? <OperatorPanel p={p} sendCommand={sendCommand} />
           : <p className="prompt-desc">Waiting for the machine…</p>}
      </div>

      <p className="section-label">Outputs · only one runs at a time</p>
      <div className="kpi-grid" role="region" aria-label="Machine outputs">
        {OUTPUTS.map(o => {
          const [label, variant] = outputDisplay(o.key, outputs[o.key], p);
          return (
            <KpiCard key={o.key} id={`out-${o.key}`} label={o.name} value={label}
                     unit={o.hw} variant={variant} loading={loading} />
          );
        })}
        <KpiCard id="out-loadcell" label="Load cell" unit="S-type + HX711"
                 value={pressActive ? `${p.force_n} N` : 'IDLE'}
                 variant={pressActive ? 'accent' : ''} loading={loading} />
      </div>

      <p className="section-label">Live Readings</p>
      <div className="kpi-grid" role="region" aria-label="Live readings">
        <KpiCard id="card-water" icon={<FiDroplet />} label="Water"
                 value={p ? `${p.flow_l.toFixed(2)} L` : '—'}
                 unit={p ? `Target ${p.target_l.toFixed(2)} L (${p.water_pct}%)` : 'Flow meter'}
                 variant="info" loading={loading} />
        <KpiCard id="card-force" icon={<FiActivity />} label="Press force"
                 value={p ? `${p.force_n} N` : '—'}
                 unit={p ? `Peak ${p.peak_force_n} N · target ${CONFIG.PRESS_TARGET_N} N` : 'Load cell'}
                 variant={p?.force_n >= CONFIG.PRESS_TARGET_N ? 'accent' : ''} loading={loading} />
        <KpiCard id="card-mixer" icon={<FiZap />} label="Mixer current"
                 value={p ? `${p.mixer_a.toFixed(1)} A` : '—'} unit="20V motor, 5A max"
                 loading={loading} />
        <KpiCard id="card-batch" icon={<FiClock />} label="Batch time"
                 value={p ? fmtMMSS(p.batch_ms) : '—'} unit="Water start to unload"
                 loading={loading} />
      </div>

      <p className="section-label">Operational Diagnostics Log</p>
      <EventLog entries={logEntries} />
    </>
  );
}
