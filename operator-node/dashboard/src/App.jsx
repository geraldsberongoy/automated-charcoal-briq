// App.jsx — React PWA Dashboard for the AUTOBRIQ fully automated briquette machine
import { useState, useEffect, useRef } from 'react';

import { useStats }  from './hooks/useStats';
import Navbar        from './components/Navbar';
import KpiCard       from './components/KpiCard';
import InfoPanel     from './components/InfoPanel';
import HeapBar       from './components/HeapBar';
import EventLog      from './components/EventLog';
import Badge         from './components/Badge';
import BatchWorkspace from './components/BatchWorkspace';
import {
  FiCpu,
  FiBatteryCharging,
  FiDatabase,
  FiSun,
  FiZap,
  FiWifi,
  FiLayers,
  FiSettings,
  FiKey,
  FiAlertTriangle,
  FiDownload,
  FiRotateCcw,
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

// Mock historical logs for tab 3
const MOCK_RUNS = [
  { id: 125, timestamp: '2026-10-14 16:15', level: '100%', water: '3.00 L', mix: '03:00', force: '2006 N', total: '05:41', status: 'OK' },
  { id: 124, timestamp: '2026-10-14 15:40', level: '80%',  water: '2.40 L', mix: '03:00', force: '2004 N', total: '05:38', status: 'OK' },
  { id: 123, timestamp: '2026-10-14 15:05', level: '100%', water: '3.00 L', mix: '03:00', force: '1450 N', total: '—',     status: 'STOPPED' },
  { id: 122, timestamp: '2026-10-14 14:15', level: '90%',  water: '2.70 L', mix: '03:00', force: '2009 N', total: '05:40', status: 'OK' },
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

  // Generate log entries when stats change (step transitions, client events)
  useEffect(() => {
    if (!stats) return;
    const prev = prevStatsRef.current;

    if (prev) {
      const step = stats.process?.step;
      if (step && step !== prev.process?.step) {
        if (step === 'STOPPED') {
          addLog('err', `Process STOPPED during ${stats.process.stopped_during} — all relays off`);
        } else {
          addLog('ok', `Step → ${step}`);
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
      {/* ── Process stopped overlay (STOP pressed / safety cover opened) ─── */}
      {stats?.process?.step === 'STOPPED' && (
        <div className="safety-overlay">
          <div className="safety-box">
            <div className="safety-icon"><FiAlertTriangle /></div>
            <div className="safety-title">Process Stopped</div>
            <div className="safety-desc">
              STOP was pressed or the safety cover was opened during{' '}
              <b>{stats.process.stopped_during}</b>. All relays are switched off.
              Reset retracts the press and closes the gate before the next batch.
            </div>
            <button className="action-btn danger" onClick={() => sendCommand('RESET')}>
              <FiRotateCcw /> Reset &amp; retract
            </button>
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

        {/* ── Tab 1: Operator Workspace (batch flow) ─────────────────────── */}
        {activeTab === 'workspace' && (
          <BatchWorkspace
            stats={stats}
            loading={loading}
            sendCommand={sendCommand}
            logEntries={logEntries}
          />
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
                    <th>Water Level</th>
                    <th>Water Vol</th>
                    <th>Mixing Time</th>
                    <th>Peak Force</th>
                    <th>Total Time</th>
                    <th>Run Status</th>
                  </tr>
                </thead>
                <tbody>
                  {MOCK_RUNS.map(run => (
                    <tr key={run.id}>
                      <td>#{run.id}</td>
                      <td>{run.timestamp}</td>
                      <td>{run.level}</td>
                      <td>{run.water}</td>
                      <td>{run.mix}</td>
                      <td>{run.force}</td>
                      <td>{run.total}</td>
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

