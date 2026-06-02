// App.jsx — Upgraded React PWA Dashboard for the Semi-Automated Briquette Machine
import { useState, useEffect, useRef } from 'react';

import { useStats }  from './hooks/useStats';
import Navbar        from './components/Navbar';
import KpiCard       from './components/KpiCard';
import InfoPanel     from './components/InfoPanel';
import HeapBar       from './components/HeapBar';
import EventLog      from './components/EventLog';
import Badge         from './components/Badge';
import { 
  FiCpu, 
  FiBatteryCharging, 
  FiDatabase, 
  FiActivity, 
  FiDroplet, 
  FiThermometer, 
  FiWind, 
  FiSun, 
  FiZap, 
  FiWifi, 
  FiLayers, 
  FiSettings, 
  FiKey, 
  FiAlertTriangle, 
  FiDownload,
  FiPlay 
} from 'react-icons/fi';

// ── Helpers ───────────────────────────────────────────────────────────────────
function fmtHMS(ms) {
  if (!ms && ms !== 0) return '—';
  const s = Math.floor(ms / 1000);
  const h = Math.floor(s / 3600);
  const m = Math.floor((s % 3600) / 60);
  const sec = s % 60;
  return [h, m, sec].map(v => String(v).padStart(2, '0')).join(':');
}

function fmtKB(bytes) {
  if (bytes == null) return '—';
  return `${(bytes / 1024).toFixed(1)} KB`;
}

// ── Initial boot log entries ──────────────────────────────────────────────────
const BOOT_LOG = [
  { level: 'ok',   msg: 'System Boot — FSM mounted on LittleFS partition',  time: new Date() },
  { level: 'ok',   msg: 'SoftAP initialized — SSID: Operator-Node-01',     time: new Date() },
  { level: 'ok',   msg: 'Wireless Dashboard connected via local edge node',  time: new Date() },
  { level: 'info', msg: 'Captive portal active on port 80 — redirect Mode: 302', time: new Date() },
];

const FSM_STEPS = [
  { num: 1, name: 'Soaking', desc: 'Water Flow (3.0L)' },
  { num: 2, name: 'Grinding', desc: 'Fiber Pulverize' },
  { num: 3, name: 'Mixing', desc: 'Slurry Homogenize' },
  { num: 4, name: 'Compaction', desc: 'Press (2000N)' },
  { num: 5, name: 'Drying', desc: 'Curing (<=15% RH)' }
];

// Mock historical logs for tab 3
const MOCK_RUNS = [
  { id: 125, timestamp: '2026-06-03 00:15', grind: '12s', water: '3.00 L', mix: '30s', force: '2005 N', dry: '420s', rh: '14.8 %', status: 'OK' },
  { id: 124, timestamp: '2026-06-02 23:40', grind: '12s', water: '2.98 L', mix: '30s', force: '2008 N', dry: '415s', rh: '15.0 %', status: 'OK' },
  { id: 123, timestamp: '2026-06-02 23:05', grind: '12s', water: '3.00 L', mix: '10s', force: '850 N',  dry: '—',    rh: '—',      status: 'FAULT' },
  { id: 122, timestamp: '2026-06-02 22:15', grind: '12s', water: '3.02 L', mix: '30s', force: '2012 N', dry: '435s', rh: '14.5 %', status: 'OK' }
];

export default function App() {
  const { stats, loading, error, lastUpdated, connectionType, sendCommand } = useStats();
  const [logEntries, setLogEntries] = useState(BOOT_LOG);
  const [activeTab, setActiveTab] = useState('workspace'); // workspace, power, logs
  const prevStatsRef = useRef(null);

  const [isAdmin, setIsAdmin] = useState(false);
  const [showPinModal, setShowPinModal] = useState(false);
  const [pinInput, setPinInput] = useState('');
  const [pinError, setPinError] = useState('');
  const [pendingAction, setPendingAction] = useState(null);

  function handleToggleAdmin() {
    if (isAdmin) {
      setIsAdmin(false);
      addLog('info', 'Admin session locked. Returned to standard Operator role.');
      if (activeTab === 'logs') {
        setActiveTab('workspace');
      }
    } else {
      setShowPinModal(true);
      setPinError('');
      setPinInput('');
      setPendingAction(null);
    }
  }

  function handlePinSubmit() {
    if (pinInput === '1234') {
      setIsAdmin(true);
      setShowPinModal(false);
      setPinError('');
      addLog('ok', 'Admin Mode authenticated successfully.');
      if (pendingAction) {
        pendingAction();
        setPendingAction(null);
      }
    } else {
      setPinError('INVALID PASSCODE. ACCESS DENIED.');
      setPinInput('');
    }
  }

  function runAdminAction(actionCallback) {
    if (isAdmin) {
      actionCallback();
    } else {
      setPendingAction(() => actionCallback);
      setShowPinModal(true);
      setPinError('');
      setPinInput('');
    }
  }

  // Generate log entries when stats change (state transitions, client events, safety faults)
  useEffect(() => {
    if (!stats) return;
    const prev = prevStatsRef.current;

    if (prev) {
      if (stats.active_state !== prev.active_state) {
        const stepName = FSM_STEPS.find(s => s.num === stats.active_state)?.name || 'Unknown';
        addLog('ok', `FSM transition into State ${stats.active_state}: ${stepName}`);
      }
      
      if (stats.safety_switch !== prev.safety_switch) {
        if (stats.safety_switch === 0) {
          addLog('err', 'CRITICAL SAFETY ALERT: Limit switch triggered / casing breach detected!');
        } else {
          addLog('ok', 'Safety interlock restored. Operation resumed.');
        }
      }

      if (stats.clients > prev.clients) {
        addLog('info', `Client device connected to SoftAP — total clients: ${stats.clients}`);
      } else if (stats.clients < prev.clients) {
        addLog('warn', `Client device disconnected — total clients: ${stats.clients}`);
      }
    }

    prevStatsRef.current = stats;
  }, [stats]);

  // Log connection errors
  useEffect(() => {
    if (error) addLog('err', `AP Telemetry error: ${error}`);
  }, [error]);

  function addLog(level, msg) {
    setLogEntries(prev => [
      ...prev.slice(-99), // keep last 100 entries
      { level, msg, time: new Date() },
    ]);
  }

  // ── Derived display values ──────────────────────────────────────────────────
  const apRows = stats ? [
    { key: 'SSID',        value: stats.ssid     ?? 'Operator-Node-01' },
    { key: 'Security',    value: <Badge variant="green">{stats.security ?? 'WPA2-PSK'}</Badge> },
    { key: 'IP Address',  value: stats.ip        ?? '192.168.4.1' },
    { key: 'Subnet Mask', value: stats.subnet    ?? '255.255.255.0' },
    { key: 'Gateway',     value: stats.gateway   ?? '192.168.4.1' },
    { key: 'MAC Address', value: stats.mac       ?? '—' },
  ] : [];

  const svcRows = [
    { key: 'DNS Server',    value: <Badge variant="green">✓ UDP :53 (Captive Redirect)</Badge> },
    { key: 'HTTP Server',   value: <Badge variant="green">✓ TCP :80</Badge> },
    { key: 'Storage Type',  value: <Badge variant="blue">LittleFS (Flash)</Badge> },
    { key: 'Run Logger',    value: <Badge variant="green">✓ Micro-SD (CSV)</Badge> },
    { key: 'Captive Portal',value: <Badge variant="green">✓ ACTIVE</Badge> },
  ];

  const sysRows = stats ? [
    { key: 'CPU Frequency', value: stats.cpu_freq_mhz ? `${stats.cpu_freq_mhz} MHz` : '240 MHz' },
    { key: 'Free Heap',     value: fmtKB(stats.free_heap) },
    { key: 'Total Heap',    value: fmtKB(stats.total_heap ?? 327680) },
    { key: 'Wi-Fi Channel', value: `Ch ${stats.channel ?? 6} (802.11 b/g/n)` },
    { key: 'Last Update',   value: lastUpdated ? lastUpdated.toLocaleTimeString() : '—' },
  ] : [];

  return (
    <>
      {/* ── Safety interlock fault overlay (CRITICAL SAFETY SHUTDOWN) ─────── */}
      {stats && stats.safety_switch === 0 && (
        <div className="safety-overlay">
          <div className="safety-box">
            <div className="safety-icon"><FiAlertTriangle /></div>
            <div className="safety-title">Safety Breach Detected</div>
            <div className="safety-desc">
              The casing safety interlock switch has been opened. The compactor linear actuator duty cycle has been set to 0% to prevent mechanical hazard.
            </div>
            <Badge variant="red">CRITICAL HALT STATE</Badge>
          </div>
        </div>
      )}

      <Navbar
        error={error}
        lastUpdated={lastUpdated}
        ip={stats?.ip}
        connectionType={connectionType}
        isAdmin={isAdmin}
        onToggleAdmin={handleToggleAdmin}
      />

      <main id="dashboard" role="main">
        {/* ── Navigation Tabs ─────────────────────────────────────────────── */}
        <div className="tabs-container">
          <button
            className={`tab-btn ${activeTab === 'workspace' ? 'active' : ''}`}
            onClick={() => setActiveTab('workspace')}
          >
            <FiCpu /> <span className="tab-btn__text">Workspace</span>
          </button>
          <button
            className={`tab-btn ${activeTab === 'power' ? 'active' : ''}`}
            onClick={() => setActiveTab('power')}
          >
            <FiBatteryCharging /> <span className="tab-btn__text">System & Power</span>
          </button>
          <button
            className={`tab-btn ${activeTab === 'logs' ? 'active' : ''}`}
            onClick={() => runAdminAction(() => setActiveTab('logs'))}
          >
            <FiDatabase /> <span className="tab-btn__text">Production Logs</span>
          </button>
        </div>

        {/* ── Tab 1: Operator Workspace ──────────────────────────────────── */}
        {activeTab === 'workspace' && (
          <>
            {/* Wizard Steps Flow */}
            <p className="section-label">Finite State Machine (FSM) Progress</p>
            <div className="wizard">
              {FSM_STEPS.map((step, idx) => {
                const isActive = stats?.active_state === step.num;
                const isCompleted = stats?.active_state > step.num;
                return (
                  <div key={step.num} className={`wizard-step ${isActive ? 'active' : ''} ${isCompleted ? 'completed' : ''}`}>
                    <div className="wizard-step__circle">
                      {isCompleted ? '✓' : step.num}
                    </div>
                    <div className="wizard-step__info">
                      <span className="wizard-step__name">{step.name}</span>
                      <span className="wizard-step__desc">{step.desc}</span>
                    </div>
                    {idx < FSM_STEPS.length - 1 && (
                      <div className={`wizard-connector ${isCompleted ? 'completed' : ''}`} />
                    )}
                  </div>
                );
              })}
            </div>

            {/* Active Workflow Confirm Panel */}
            <p className="section-label">Operator Work Instructions & Gates</p>
            <div className="prompt-card active-step">
              {stats?.active_state === 1 && (
                <>
                  <div className="prompt-title">Stage 1: Paper Soaking Phase</div>
                  <p className="prompt-desc">
                    Solenoid water valve is open, injecting moisture into the waste tank to soak the raw paper waste. Monitoring water volume intake. Target: 3.00 L.
                  </p>
                  <div className="gauge-wrap">
                    <div className="gauge-label">
                      <span>Volumetric Flow (Target: 3.00 L)</span>
                      <span>{stats?.sensors?.volumetric_flow_l || 0} L</span>
                    </div>
                    <div className="gauge-track">
                      <div className="gauge-fill" style={{ width: `${Math.min(100, ((stats?.sensors?.volumetric_flow_l || 0) / 3) * 100)}%` }} />
                    </div>
                  </div>
                  <button
                    className="action-btn"
                    disabled={parseFloat(stats?.sensors?.volumetric_flow_l) < 3.0}
                    onClick={() => sendCommand('SET_STATE', { state: 2 })}
                  >
                    Confirm Water Fill Complete
                  </button>
                </>
              )}
              {stats?.active_state === 2 && (
                <>
                  <div className="prompt-title">Stage 2: Paper Slurry Grinding</div>
                  <p className="prompt-desc">
                    The 12V 775 high-speed grinder is active, pulverizing the soaked paper waste into fine fiber pulp.
                  </p>
                  <div className="gauge-wrap">
                    <div className="gauge-label">
                      <span>Grinder Current (Target: ~4.0 A)</span>
                      <span>{stats?.sensors?.grinder_current_a || 0} A</span>
                    </div>
                    <div className="gauge-track">
                      <div className="gauge-fill" style={{ width: `${Math.min(100, ((stats?.sensors?.grinder_current_a || 0) / 6) * 100)}%` }} />
                    </div>
                  </div>
                  <button
                    className="action-btn"
                    onClick={() => sendCommand('SET_STATE', { state: 3 })}
                  >
                    Confirm Grinding Completed
                  </button>
                </>
              )}
              {stats?.active_state === 3 && (
                <>
                  <div className="prompt-title">Stage 3: Slurry & Charcoal Mixing</div>
                  <p className="prompt-desc">
                    PLANETARY MIXER MOTOR ACTIVE. The mechanical mixer is blending the charcoal dust binder with the lignocellulosic paper pulp slurry. Please ensure even consistency.
                  </p>
                  <button
                    className="action-btn"
                    onClick={() => sendCommand('SET_STATE', { state: 4 })}
                  >
                    Confirm Mixing Completed
                  </button>
                </>
              )}
              {stats?.active_state === 4 && (
                <>
                  <div className="prompt-title">Stage 4: Molding Compaction</div>
                  <p className="prompt-desc">
                    BTS7960 COMPACTOR ACTUATOR DRIVING DOWN. Applying compaction force to mold the rectangular briquette chunk. Keep hands clear! Target: 2000N.
                  </p>
                  <div className="gauge-wrap">
                    <div className="gauge-label">
                      <span>Compaction Load (Target: 2000 N)</span>
                      <span>{stats?.sensors?.compaction_force_n || 0} N</span>
                    </div>
                    <div className="gauge-track">
                      <div className="gauge-fill" style={{ width: `${Math.min(100, ((stats?.sensors?.compaction_force_n || 0) / 2000) * 100)}%` }} />
                    </div>
                  </div>
                  <button
                    className="action-btn"
                    disabled={stats?.sensors?.compaction_force_n < 2000}
                    onClick={() => sendCommand('SET_STATE', { state: 5 })}
                  >
                    {stats?.sensors?.compaction_force_n >= 2000 ? 'Target Reached! Confirm Compaction' : 'Awaiting 2000N Pressure'}
                  </button>
                </>
              )}
              {stats?.active_state === 5 && (
                <>
                  <div className="prompt-title">Stage 5: Thermal Curing & Drying</div>
                  <p className="prompt-desc">
                    Halogen thermal arrays and exhaust ventilation fans are active. Drying wet briquette structure down to target &lt;= 15% exhaust relative humidity.
                  </p>
                  <div className="gauge-wrap">
                    <div className="gauge-label">
                      <span>Exhaust Relative Humidity (Target: &lt;= 15.0%)</span>
                      <span>{stats?.sensors?.relative_humidity || 0}%</span>
                    </div>
                    <div className="gauge-track">
                      <div
                        className={`gauge-fill ${parseFloat(stats?.sensors?.relative_humidity) <= 15 ? 'success' : 'warning'}`}
                        style={{ width: `${Math.max(15, stats?.sensors?.relative_humidity || 0)}%` }}
                      />
                    </div>
                  </div>
                  <button
                    className="action-btn"
                    disabled={parseFloat(stats?.sensors?.relative_humidity) > 15}
                    onClick={() => sendCommand('SET_STATE', { state: 1 })}
                  >
                    Unload Briquette & Terminate Run
                  </button>
                </>
              )}
            </div>

            {/* Live Telemetry KPI Cards */}
            <p className="section-label">Active Sensory Telemetry</p>
            <div className="kpi-grid" role="region" aria-label="Live sensors">
              <KpiCard
                id="card-force"
                icon={<FiActivity />}
                label="Compaction Force"
                value={stats?.sensors?.compaction_force_n ? `${stats.sensors.compaction_force_n} N` : '—'}
                unit="S-Type Load Cell"
                variant={stats?.sensors?.compaction_force_n >= 2000 ? 'accent' : ''}
                loading={loading}
              />
              <KpiCard
                id="card-flow"
                icon={<FiDroplet />}
                label="Volumetric Inflow"
                value={stats?.sensors?.volumetric_flow_l ? `${stats.sensors.volumetric_flow_l} L` : '—'}
                unit="YF-S201 Flow Meter"
                variant={stats?.sensors?.volumetric_flow_l >= 3.0 ? 'info' : ''}
                loading={loading}
              />
              <KpiCard
                id="card-temp"
                icon={<FiThermometer />}
                label="Curing Enclosure Temp"
                value={stats?.sensors?.curing_temp_c ? `${stats.sensors.curing_temp_c} °C` : '—'}
                unit="DS18B20 Enclosure Array"
                loading={loading}
              />
              <KpiCard
                id="card-humidity"
                icon={<FiWind />}
                label="Exhaust Humidity"
                value={stats?.sensors?.relative_humidity ? `${stats.sensors.relative_humidity} %` : '—'}
                unit="DHT22 Sensor"
                variant={stats?.sensors?.relative_humidity <= 15 ? 'accent' : 'warn'}
                loading={loading}
              />
            </div>

            {/* Nextion HMI Simulator */}
            <p className="section-label">On-Machine Nextion Touchscreen Simulator</p>
            <div className="nextion-simulator">
              <div className="nextion-bezel">
                <div className="nextion-lcd">
                  {stats && stats.safety_switch === 0 ? (
                    // Safety Alert Page
                    <div style={{ display: 'flex', flexDirection: 'column', height: '100%', justifyContent: 'space-between' }}>
                      <div className="nextion-lcd-header" style={{ color: 'var(--clr-danger)' }}>
                        <span>PAGE: fault</span>
                        <span>[ BAT: {stats?.power?.battery_voltage_v ? `${stats.power.battery_voltage_v}V` : '—'} ]</span>
                      </div>
                      <div className="nextion-lcd-body">
                        <div className="nextion-lcd-alert">
                          <div>CRITICAL SAFETY HALT</div>
                          <div style={{ fontSize: '0.6rem', marginTop: '0.25rem' }}>
                            CASING SWITCH TRIGGERED — MOTOR RELAYS OPENED
                          </div>
                        </div>
                      </div>
                      <div className="nextion-lcd-footer">
                        <span style={{ fontSize: '0.6rem', color: 'var(--clr-text-2)' }}>Nextion 4.3" Intelligent HUD</span>
                        <button className="nextion-btn" style={{ background: 'var(--clr-danger)', color: '#fff', borderColor: 'var(--clr-danger)' }}>
                          RESET FAULT
                        </button>
                      </div>
                    </div>
                  ) : (
                    // Normal Pages (State 1 to 5)
                    <div style={{ display: 'flex', flexDirection: 'column', height: '100%', justifyContent: 'space-between' }}>
                      <div className="nextion-lcd-header">
                        <span>PAGE: {
                          stats?.active_state === 1 ? 'soak' :
                          stats?.active_state === 2 ? 'grind' :
                          stats?.active_state === 3 ? 'mix' :
                          stats?.active_state === 4 ? 'mold' : 'dry'
                        }</span>
                        <span>[ BAT: {stats?.power?.battery_voltage_v ? `${stats.power.battery_voltage_v}V` : '—'} | PV: {stats?.power?.solar_amperage_a ? `${stats.power.solar_amperage_a}A` : '—'} ]</span>
                      </div>
                      
                      <div className="nextion-lcd-body">
                        {stats?.active_state === 1 && (
                          <div style={{ display: 'flex', flexDirection: 'column', gap: '6px' }}>
                            <div style={{ display: 'flex', justifyContent: 'space-between' }}>
                              <span>SOLENOID: <span style={{ color: 'var(--clr-accent)' }}>OPEN</span></span>
                              <span>GRINDER: <span style={{ color: 'var(--clr-text-2)' }}>OFF</span></span>
                            </div>
                            <div>WATER INTAKE: {stats.sensors.volumetric_flow_l} L / 3.00 L</div>
                            <div className="gauge-track" style={{ height: '6px' }}>
                              <div className="gauge-fill" style={{ width: `${Math.min(100, (stats.sensors.volumetric_flow_l / 3.00) * 100)}%` }} />
                            </div>
                          </div>
                        )}
                        {stats?.active_state === 2 && (
                          <div style={{ display: 'flex', flexDirection: 'column', gap: '6px' }}>
                            <div style={{ display: 'flex', justifyContent: 'space-between' }}>
                              <span>GRINDER: <span style={{ color: 'var(--clr-accent)' }}>RUNNING</span></span>
                              <span>CURRENT: <span style={{ color: 'var(--clr-accent)' }}>{stats.sensors.grinder_current_a} A</span></span>
                            </div>
                            <div className="gauge-track" style={{ height: '6px' }}>
                              <div className="gauge-fill" style={{ width: `${Math.min(100, (stats.sensors.grinder_current_a / 6.0) * 100)}%` }} />
                            </div>
                          </div>
                        )}
                        {stats?.active_state === 3 && (
                          <div style={{ display: 'flex', flexDirection: 'column', gap: '6px' }}>
                            <div>MIXER STATE: <span style={{ color: 'var(--clr-accent)' }}>ACTIVE</span> (60 RPM)</div>
                            <div>TIMER COUNTDOWN: 00:18</div>
                            <div className="gauge-track" style={{ height: '6px' }}>
                              <div className="gauge-fill" style={{ width: '40%' }} />
                            </div>
                          </div>
                        )}
                        {stats?.active_state === 4 && (
                          <div style={{ display: 'flex', flexDirection: 'column', gap: '6px' }}>
                            <div>FORCE: {stats.sensors.compaction_force_n} N / 2000 N</div>
                            <div className="gauge-track" style={{ height: '6px' }}>
                              <div className="gauge-fill" style={{ width: `${Math.min(100, (stats.sensors.compaction_force_n / 2000) * 100)}%` }} />
                            </div>
                            <div style={{ display: 'flex', gap: '4px', marginTop: '2px' }}>
                              <button className="nextion-btn" style={{ flex: 1 }} disabled>EXTEND</button>
                              <button className="nextion-btn" style={{ flex: 1 }} disabled>RETRACT</button>
                            </div>
                          </div>
                        )}
                        {stats?.active_state === 5 && (
                          <div style={{ display: 'flex', flexDirection: 'column', gap: '6px' }}>
                            <div>DRYER: <span style={{ color: 'var(--clr-accent)' }}>ACTIVE</span> (50W THERMAL)</div>
                            <div>TEMP: {stats.sensors.curing_temp_c} C | HUMID: {stats.sensors.relative_humidity}%</div>
                            <div className="gauge-track" style={{ height: '6px' }}>
                              <div className="gauge-fill" style={{ width: `${Math.max(15, stats.sensors.relative_humidity)}%` }} />
                            </div>
                          </div>
                        )}
                      </div>

                      <div className="nextion-lcd-footer">
                        <span style={{ fontSize: '0.6rem', color: 'var(--clr-text-2)' }}>Nextion 4.3" HMI</span>
                        {stats?.active_state === 1 && (
                          <button
                            className="nextion-btn accent"
                            disabled={stats.sensors.volumetric_flow_l < 3.0}
                            onClick={() => sendCommand('SET_STATE', { state: 2 })}
                          >
                            SOAK DONE
                          </button>
                        )}
                        {stats?.active_state === 2 && (
                          <button
                            className="nextion-btn accent"
                            onClick={() => sendCommand('SET_STATE', { state: 3 })}
                          >
                            GRIND DONE
                          </button>
                        )}
                        {stats?.active_state === 3 && (
                          <button
                            className="nextion-btn accent"
                            onClick={() => sendCommand('SET_STATE', { state: 4 })}
                          >
                            MIX DONE
                          </button>
                        )}
                        {stats?.active_state === 4 && (
                          <button
                            className="nextion-btn accent"
                            disabled={stats.sensors.compaction_force_n < 2000}
                            onClick={() => sendCommand('SET_STATE', { state: 5 })}
                          >
                            CONFIRM MOLD
                          </button>
                        )}
                        {stats?.active_state === 5 && (
                          <button
                            className="nextion-btn accent"
                            disabled={stats.sensors.relative_humidity > 15.0}
                            onClick={() => sendCommand('SET_STATE', { state: 1 })}
                          >
                            UNLOAD CHUNK
                          </button>
                        )}
                      </div>
                    </div>
                  )}
                </div>
                <div className="nextion-brand">NEXTION</div>
              </div>
            </div>

            {/* Hardware Override Simulation Panel */}
            <p className="section-label">Hardware Override Controls (Testing & Simulation)</p>
            <div style={{
              background: 'var(--clr-surface)',
              border: '1px solid var(--clr-border)',
              borderRadius: 'var(--radius-md)',
              padding: '1.25rem 1.5rem',
              boxShadow: 'var(--shadow-card)',
              marginBottom: '2rem',
              display: 'flex',
              flexWrap: 'wrap',
              gap: '12px',
              alignItems: 'center'
            }}>
              <div style={{ flex: '1 1 200px' }}>
                <div style={{ fontWeight: 600, fontSize: '0.85rem' }}>Casing Safety Switch Status</div>
                <div style={{ fontSize: '0.72rem', color: 'var(--clr-text-2)', marginTop: '2px' }}>
                  Simulates opening the machine cover casing.
                </div>
              </div>
              <div style={{ display: 'flex', gap: '8px' }}>
                {stats?.safety_switch === 1 ? (
                  <button
                    className="nextion-btn"
                    style={{ background: 'var(--clr-danger)', color: '#fff', borderColor: 'var(--clr-danger)' }}
                    onClick={() => runAdminAction(() => sendCommand('TRIP_SAFETY'))}
                  >
                    🚨 Trip Casing (Open)
                  </button>
                ) : (
                  <button
                    className="nextion-btn accent"
                    onClick={() => runAdminAction(() => sendCommand('RESET_FAULT'))}
                  >
                    🔒 Close Casing (Reset)
                  </button>
                )}
              </div>
            </div>

            {/* Event Log */}
            <p className="section-label">Operational Diagnostics Log</p>
            <EventLog entries={logEntries} />
          </>
        )}

        {/* ── Tab 2: System & Power HUD ──────────────────────────────────── */}
        {activeTab === 'power' && (
          <>
            <p className="section-label">Solar Power infrastructure</p>
            <div className="kpi-grid" role="region" aria-label="Power Telemetry">
              <KpiCard
                id="card-voltage"
                icon={<FiBatteryCharging />}
                label="Battery Voltage"
                value={stats?.power?.battery_voltage_v ? `${stats.power.battery_voltage_v} V` : '—'}
                unit="12V 30Ah LiFePO4 Pack"
                variant="info"
                loading={loading}
              />
              <KpiCard
                id="card-solar"
                icon={<FiSun />}
                label="Solar Input Current"
                value={stats?.power?.solar_amperage_a ? `${stats.power.solar_amperage_a} A` : '—'}
                unit="100W Monocrystalline PV"
                variant="accent"
                loading={loading}
              />
              <KpiCard
                id="card-battery-soc"
                icon={<FiZap />}
                label="State of Charge"
                value={stats?.power?.battery_voltage_v ? `${Math.floor(((stats.power.battery_voltage_v - 11.5) / (14.2 - 11.5)) * 100)} %` : '—'}
                unit="Charge Capacity"
                variant={stats?.power?.battery_voltage_v > 12.8 ? 'accent' : 'warn'}
                loading={loading}
              />
            </div>

            {stats && (
              <div style={{ marginBottom: '2rem' }}>
                <p className="section-label">Battery Level Gauge</p>
                <div style={{
                  background: 'var(--clr-surface)',
                  border: '1px solid var(--clr-border)',
                  borderRadius: 'var(--radius-md)',
                  padding: '1.25rem 1.5rem',
                  boxShadow: 'var(--shadow-card)',
                }}>
                  <div className="gauge-wrap" style={{ margin: 0 }}>
                    <div className="gauge-label" style={{ marginBottom: '0.5rem' }}>
                      <span>LiFePO4 Energy Reservoir Status</span>
                      <span>{Math.floor(((stats.power.battery_voltage_v - 11.5) / 2.7) * 100)}%</span>
                    </div>
                    <div className="gauge-track" style={{ height: '14px', borderRadius: '7px' }}>
                      <div
                        className="gauge-fill"
                        style={{
                          width: `${Math.min(100, Math.max(5, Math.floor(((stats.power.battery_voltage_v - 11.5) / 2.7) * 100)))}%`,
                          background: 'linear-gradient(90deg, #10b981, var(--clr-accent))'
                        }}
                      />
                    </div>
                  </div>
                </div>
              </div>
            )}

            {/* Heap Memory Indicator */}
            {stats && (
              <div style={{ marginBottom: '2rem' }}>
                <p className="section-label">Microcontroller Heap Memory</p>
                <div style={{
                  background: 'var(--clr-surface)',
                  border: '1px solid var(--clr-border)',
                  borderRadius: 'var(--radius-md)',
                  padding: '1.25rem 1.5rem',
                  boxShadow: 'var(--shadow-card)',
                }}>
                  <HeapBar freeHeap={stats.free_heap} totalHeap={stats.total_heap ?? 327680} />
                </div>
              </div>
            )}

            {/* Network Info */}
            <p className="section-label">Edge Web Settings</p>
            <div className="info-grid" role="region" aria-label="Web Config">
              {stats && (
                <InfoPanel id="panel-ap"       icon={<FiWifi />} title="Access Point Broadcast" rows={apRows}  />
              )}
              <InfoPanel   id="panel-services" icon={<FiLayers />} title="Microcontroller Services" rows={svcRows} />
              {stats && sysRows.length > 0 && (
                <InfoPanel id="panel-system"   icon={<FiSettings />} title="Core Firmware Diagnostics" rows={sysRows} />
              )}
            </div>
          </>
        )}

        {/* ── Tab 3: Historical Production Logs ──────────────────────────── */}
        {activeTab === 'logs' && (
          <>
            <div className="logs-action-bar">
              <p className="section-label" style={{ margin: 0 }}>SD Card CSV Batch Logs</p>
              <button
                className="action-btn"
                style={{ padding: '0.4rem 1rem', fontSize: '0.78rem', display: 'flex', alignItems: 'center', gap: '0.25rem' }}
                onClick={() => alert('Downloading production_runs.csv...')}
              >
                <FiDownload /> Download CSV Log
              </button>
            </div>

            <div className="logs-table-card">
              <table className="logs-table">
                <thead>
                  <tr>
                    <th>Batch ID</th>
                    <th>Timestamp</th>
                    <th>Pulverize Time</th>
                    <th>Water Vol</th>
                    <th>Mixing Time</th>
                    <th>Compaction Load</th>
                    <th>Drying Duration</th>
                    <th>Final RH</th>
                    <th>Run Status</th>
                  </tr>
                </thead>
                <tbody>
                  {MOCK_RUNS.map(run => (
                    <tr key={run.id}>
                      <td>#{run.id}</td>
                      <td>{run.timestamp}</td>
                      <td>{run.grind}</td>
                      <td>{run.water}</td>
                      <td>{run.mix}</td>
                      <td>{run.force}</td>
                      <td>{run.dry}</td>
                      <td>{run.rh}</td>
                      <td>
                        <Badge variant={run.status === 'OK' ? 'green' : 'red'}>
                          {run.status}
                        </Badge>
                      </td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          </>
        )}

      </main>

      <footer role="contentinfo">
        ESP32 Operator Node v3.0.0 &mdash; LFS PWA Mode &mdash; local edge node &mdash; real-time ws telemetry
      </footer>

      {/* ── Admin PIN Modal ─────────────────────────────────────────────── */}
      {showPinModal && (
        <div className="safety-overlay" style={{ zIndex: 110 }}>
          <div className="safety-box" style={{ borderColor: 'var(--clr-border)', background: 'var(--clr-bg-1)', maxWidth: '360px' }}>
            <div className="safety-icon" style={{ textShadow: 'none' }}><FiKey /></div>
            <div className="safety-title" style={{ fontSize: '1.15rem' }}>Admin Authentication</div>
            <div className="safety-desc" style={{ fontSize: '0.78rem', marginBottom: '1rem' }}>
              Entering Admin Mode is required to perform system gates, manual overrides, or calibration. 
              <br />
              <span style={{ color: 'var(--clr-accent)', fontSize: '0.75rem', fontFamily: 'var(--font-mono)', fontWeight: 600 }}>Default Passcode: 1234</span>
            </div>
            
            <div style={{ display: 'flex', flexDirection: 'column', width: '100%', gap: '8px', marginBottom: '1rem' }}>
              <input
                type="password"
                maxLength={4}
                placeholder="••••"
                value={pinInput}
                onChange={(e) => {
                  const val = e.target.value.replace(/\D/g, ''); // numbers only
                  setPinInput(val);
                  setPinError('');
                }}
                onKeyDown={(e) => {
                  if (e.key === 'Enter') handlePinSubmit();
                }}
                style={{
                  background: 'var(--clr-bg-0)',
                  border: '1px solid var(--clr-border)',
                  color: '#fff',
                  borderRadius: 'var(--radius-sm)',
                  padding: '10px',
                  textAlign: 'center',
                  fontSize: '1.5rem',
                  letterSpacing: '0.4em',
                  fontFamily: 'var(--font-mono)',
                  outline: 'none',
                  width: '100%',
                }}
                autoFocus
              />
              {pinError && (
                <div style={{ color: 'var(--clr-danger)', fontSize: '0.7rem', fontFamily: 'var(--font-mono)', textAlign: 'center' }}>
                  {pinError}
                </div>
              )}
            </div>

            <div style={{ display: 'flex', width: '100%', gap: '8px' }}>
              <button
                className="nextion-btn"
                onClick={() => {
                  setShowPinModal(false);
                  setPendingAction(null);
                }}
                style={{ flex: 1, padding: '8px' }}
              >
                Cancel
              </button>
              <button
                className="nextion-btn accent"
                onClick={handlePinSubmit}
                style={{ flex: 1, padding: '8px' }}
              >
                Submit
              </button>
            </div>
          </div>
        </div>
      )}
    </>
  );
}

