// App.jsx — Root application component
// Composes all dashboard panels and manages global state (stats polling, event log)
import { useState, useEffect, useRef } from 'react';

import { useStats }  from './hooks/useStats';
import Navbar        from './components/Navbar';
import KpiCard       from './components/KpiCard';
import InfoPanel     from './components/InfoPanel';
import HeapBar       from './components/HeapBar';
import EventLog      from './components/EventLog';
import Badge         from './components/Badge';

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
  { level: 'ok',   msg: 'SoftAP initialized — SSID: Operator-Node-01',     time: new Date() },
  { level: 'ok',   msg: 'IP address assigned: 192.168.4.1',                  time: new Date() },
  { level: 'ok',   msg: 'DNS server started on UDP port 53',                 time: new Date() },
  { level: 'ok',   msg: 'HTTP server started on TCP port 80 (LittleFS)',     time: new Date() },
  { level: 'info', msg: 'Captive portal active — awaiting client connections', time: new Date() },
];

// ── App ───────────────────────────────────────────────────────────────────────
export default function App() {
  const { stats, loading, error, lastUpdated } = useStats();
  const [logEntries, setLogEntries] = useState(BOOT_LOG);
  const prevStatsRef = useRef(null);

  // Generate log entries when stats change (client joins/leaves, DNS spikes)
  useEffect(() => {
    if (!stats) return;
    const prev = prevStatsRef.current;

    if (prev) {
      if (stats.clients > prev.clients) {
        addLog('info', `Client device joined — total: ${stats.clients}`);
      } else if (stats.clients < prev.clients) {
        addLog('warn', `Client device left — total: ${stats.clients}`);
      }

      if (stats.dns_intercepts > prev.dns_intercepts) {
        const delta = stats.dns_intercepts - prev.dns_intercepts;
        if (delta > 0) {
          addLog('warn', `${delta} DNS quer${delta === 1 ? 'y' : 'ies'} intercepted — redirected to ${stats.ip}`);
        }
      }
    } else {
      // First fetch
      addLog('ok', `Dashboard connected — polling /api/stats every 5 s`);
    }

    prevStatsRef.current = stats;
  }, [stats]);

  // Log errors
  useEffect(() => {
    if (error) addLog('err', `API error: ${error}`);
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
    { key: 'DNS Server',    value: <Badge variant="green">✓ UDP :53</Badge> },
    { key: 'HTTP Server',   value: <Badge variant="green">✓ TCP :80</Badge> },
    { key: 'Storage',       value: <Badge variant="blue">LittleFS</Badge> },
    { key: 'Captive Portal',value: <Badge variant="green">✓ ACTIVE</Badge> },
    { key: 'Redirect Mode', value: <Badge variant="warn">302 FOUND</Badge> },
    { key: 'Internet',      value: <Badge variant="warn">✗ OFFLINE</Badge> },
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
      <Navbar
        error={error}
        lastUpdated={lastUpdated}
        ip={stats?.ip}
      />

      <main id="dashboard" role="main">

        {/* ── KPI Row ─────────────────────────────────────────────────────── */}
        <p className="section-label">System Telemetry fuck</p>
        <div className="kpi-grid" role="region" aria-label="Key performance indicators">
          <KpiCard
            id="card-uptime"
            icon="⏱"
            label="Node Uptime"
            value={fmtHMS(stats?.uptime_ms)}
            unit="hh:mm:ss since boot"
            variant="accent"
            loading={loading}
          />
          <KpiCard
            id="card-clients"
            icon="📡"
            label="Connected Clients"
            value={stats?.clients}
            unit="devices on AP"
            variant="info"
            loading={loading}
          />
          <KpiCard
            id="card-channel"
            icon="📶"
            label="Wi-Fi Channel"
            value={stats?.channel ?? 6}
            unit="802.11 b/g/n"
            loading={loading}
          />
          <KpiCard
            id="card-dns"
            icon="🔀"
            label="DNS Intercepts"
            value={stats?.dns_intercepts ?? 0}
            unit="queries hijacked"
            variant="warn"
            loading={loading}
          />
        </div>

        {/* ── Heap Bar ────────────────────────────────────────────────────── */}
        {stats && (
          <div style={{ marginBottom: '2rem' }}>
            <p className="section-label">Heap Memory</p>
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

        {/* ── Info Panels ─────────────────────────────────────────────────── */}
        <p className="section-label">Network Configuration</p>
        <div className="info-grid" role="region" aria-label="Network configuration panels">
          {stats && (
            <InfoPanel id="panel-ap"       icon="◲" title="Access Point"   rows={apRows}  />
          )}
          <InfoPanel   id="panel-services" icon="▶" title="Active Services" rows={svcRows} />
          {stats && sysRows.length > 0 && (
            <InfoPanel id="panel-system"   icon="⚙" title="System Info"    rows={sysRows} />
          )}
        </div>

        {/* ── Event Log ───────────────────────────────────────────────────── */}
        <p className="section-label">Event Log</p>
        <EventLog entries={logEntries} />

      </main>

      <footer role="contentinfo">
        ESP32 Operator Node v2.0.0 &mdash; LittleFS Mode &mdash; React PWA &mdash; Polls /api/stats every 5 s
      </footer>
    </>
  );
}
