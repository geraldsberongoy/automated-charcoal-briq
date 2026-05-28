#pragma once
// ============================================================================
// WebAssets.h — PROGMEM-resident HTML/CSS/JS payload for the Operator Dashboard
//
// All web UI is stored in flash (PROGMEM) using a raw string literal to keep
// C++ logic files clean and free of presentation concerns.
//
// Stored as: const char OPERATOR_DASHBOARD_HTML[] PROGMEM
// Accessed via: WebServer::send_P() or FPSTR() helper macro
// ============================================================================

#include <pgmspace.h>

// ----------------------------------------------------------------------------
// Operator Dashboard — Dark-Mode Single Page Application
//
// Features:
//  • Real-time clock (client-side JS, no server dependency)
//  • System status cards (AP info, connected clients, uptime)
//  • Responsive CSS Grid layout
//  • CSS custom properties (design tokens) for easy theming
//  • Pure vanilla JS — no frameworks, no CDN dependencies
// ----------------------------------------------------------------------------
const char OPERATOR_DASHBOARD_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <meta name="description" content="ESP32 Operator Node — Local Area Network Dashboard" />
  <title>Operator Node — Dashboard</title>

  <style>
    /* ─── Design Tokens ────────────────────────────────────────────────── */
    :root {
      --clr-bg-0:       #0a0c10;
      --clr-bg-1:       #12161e;
      --clr-bg-2:       #1a2030;
      --clr-surface:    #1e2533;
      --clr-border:     #2a3448;
      --clr-accent:     #00d4aa;
      --clr-accent-dim: #00a882;
      --clr-warn:       #f59e0b;
      --clr-danger:     #ef4444;
      --clr-text-0:     #e8edf5;
      --clr-text-1:     #9bacc8;
      --clr-text-2:     #5d7299;
      --font-mono:      'Courier New', Courier, monospace;
      --font-sans:      -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
      --radius-sm:      6px;
      --radius-md:      12px;
      --radius-lg:      18px;
      --shadow-card:    0 4px 24px rgba(0,0,0,0.45);
      --transition:     0.2s ease;
    }

    /* ─── Reset & Base ─────────────────────────────────────────────────── */
    *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

    html { scroll-behavior: smooth; }

    body {
      background-color: var(--clr-bg-0);
      color: var(--clr-text-0);
      font-family: var(--font-sans);
      min-height: 100vh;
      display: flex;
      flex-direction: column;
    }

    /* ─── Animated Grid Background ─────────────────────────────────────── */
    body::before {
      content: '';
      position: fixed;
      inset: 0;
      background-image:
        linear-gradient(var(--clr-border) 1px, transparent 1px),
        linear-gradient(90deg, var(--clr-border) 1px, transparent 1px);
      background-size: 40px 40px;
      opacity: 0.25;
      pointer-events: none;
      z-index: 0;
    }

    /* ─── Top Navigation Bar ────────────────────────────────────────────── */
    .navbar {
      position: sticky;
      top: 0;
      z-index: 100;
      background: rgba(18, 22, 30, 0.85);
      backdrop-filter: blur(12px);
      -webkit-backdrop-filter: blur(12px);
      border-bottom: 1px solid var(--clr-border);
      padding: 0 1.5rem;
      display: flex;
      align-items: center;
      justify-content: space-between;
      height: 60px;
    }

    .navbar__brand {
      display: flex;
      align-items: center;
      gap: 0.75rem;
    }

    .navbar__icon {
      width: 32px;
      height: 32px;
      background: linear-gradient(135deg, var(--clr-accent), var(--clr-accent-dim));
      border-radius: var(--radius-sm);
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 1rem;
    }

    .navbar__title {
      font-size: 1rem;
      font-weight: 700;
      letter-spacing: 0.05em;
      color: var(--clr-text-0);
    }

    .navbar__subtitle {
      font-size: 0.7rem;
      color: var(--clr-text-2);
      font-family: var(--font-mono);
    }

    .navbar__status {
      display: flex;
      align-items: center;
      gap: 0.5rem;
      font-size: 0.8rem;
      color: var(--clr-accent);
      font-family: var(--font-mono);
    }

    .status-dot {
      width: 8px; height: 8px;
      border-radius: 50%;
      background: var(--clr-accent);
      animation: pulse 2s infinite;
    }

    @keyframes pulse {
      0%, 100% { opacity: 1; box-shadow: 0 0 0 0 rgba(0,212,170,0.4); }
      50%       { opacity: 0.7; box-shadow: 0 0 0 6px rgba(0,212,170,0); }
    }

    /* ─── Main Layout ───────────────────────────────────────────────────── */
    main {
      position: relative;
      z-index: 1;
      flex: 1;
      padding: 2rem 1.5rem;
      max-width: 1200px;
      margin: 0 auto;
      width: 100%;
    }

    /* ─── Section Header ────────────────────────────────────────────────── */
    .section-label {
      font-size: 0.7rem;
      font-family: var(--font-mono);
      color: var(--clr-accent);
      letter-spacing: 0.15em;
      text-transform: uppercase;
      margin-bottom: 1rem;
    }

    /* ─── KPI Cards Grid ────────────────────────────────────────────────── */
    .kpi-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
      gap: 1rem;
      margin-bottom: 2rem;
    }

    .kpi-card {
      background: var(--clr-surface);
      border: 1px solid var(--clr-border);
      border-radius: var(--radius-md);
      padding: 1.25rem 1.5rem;
      box-shadow: var(--shadow-card);
      transition: border-color var(--transition), transform var(--transition);
      position: relative;
      overflow: hidden;
    }

    .kpi-card:hover {
      border-color: var(--clr-accent);
      transform: translateY(-2px);
    }

    .kpi-card::before {
      content: '';
      position: absolute;
      top: 0; left: 0; right: 0;
      height: 2px;
      background: linear-gradient(90deg, var(--clr-accent), transparent);
    }

    .kpi-card__label {
      font-size: 0.7rem;
      color: var(--clr-text-2);
      font-family: var(--font-mono);
      letter-spacing: 0.1em;
      text-transform: uppercase;
      margin-bottom: 0.5rem;
    }

    .kpi-card__value {
      font-size: 1.75rem;
      font-weight: 700;
      color: var(--clr-text-0);
      font-family: var(--font-mono);
      line-height: 1;
    }

    .kpi-card__value.accent { color: var(--clr-accent); }
    .kpi-card__value.warn   { color: var(--clr-warn); }

    .kpi-card__unit {
      font-size: 0.75rem;
      color: var(--clr-text-1);
      margin-top: 0.25rem;
    }

    /* ─── Info Panel ────────────────────────────────────────────────────── */
    .info-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(320px, 1fr));
      gap: 1rem;
      margin-bottom: 2rem;
    }

    .panel {
      background: var(--clr-surface);
      border: 1px solid var(--clr-border);
      border-radius: var(--radius-md);
      box-shadow: var(--shadow-card);
      overflow: hidden;
    }

    .panel__header {
      padding: 1rem 1.5rem;
      border-bottom: 1px solid var(--clr-border);
      font-size: 0.85rem;
      font-weight: 600;
      color: var(--clr-text-0);
      display: flex;
      align-items: center;
      gap: 0.5rem;
    }

    .panel__icon { color: var(--clr-accent); }

    .panel__body { padding: 1.25rem 1.5rem; }

    .info-row {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 0.5rem 0;
      border-bottom: 1px solid var(--clr-border);
      font-size: 0.85rem;
    }

    .info-row:last-child { border-bottom: none; }

    .info-row__key {
      color: var(--clr-text-1);
      font-family: var(--font-mono);
      font-size: 0.78rem;
    }

    .info-row__val {
      color: var(--clr-text-0);
      font-family: var(--font-mono);
      font-size: 0.82rem;
      font-weight: 600;
    }

    .badge {
      display: inline-flex;
      align-items: center;
      gap: 4px;
      padding: 2px 8px;
      border-radius: 99px;
      font-size: 0.7rem;
      font-family: var(--font-mono);
      font-weight: 700;
    }

    .badge--green { background: rgba(0,212,170,0.15); color: var(--clr-accent); border: 1px solid rgba(0,212,170,0.3); }
    .badge--warn  { background: rgba(245,158,11,0.15); color: var(--clr-warn);   border: 1px solid rgba(245,158,11,0.3); }

    /* ─── Log Console ───────────────────────────────────────────────────── */
    .console {
      background: var(--clr-bg-1);
      border: 1px solid var(--clr-border);
      border-radius: var(--radius-md);
      box-shadow: var(--shadow-card);
      overflow: hidden;
      margin-bottom: 2rem;
    }

    .console__header {
      padding: 0.75rem 1.5rem;
      border-bottom: 1px solid var(--clr-border);
      font-size: 0.8rem;
      font-family: var(--font-mono);
      color: var(--clr-text-1);
      display: flex;
      align-items: center;
      justify-content: space-between;
    }

    .console__dots { display: flex; gap: 6px; }
    .console__dot {
      width: 10px; height: 10px; border-radius: 50%;
    }
    .console__dot:nth-child(1) { background: #ef4444; }
    .console__dot:nth-child(2) { background: var(--clr-warn); }
    .console__dot:nth-child(3) { background: var(--clr-accent); }

    .console__body {
      padding: 1rem 1.5rem;
      font-family: var(--font-mono);
      font-size: 0.78rem;
      line-height: 1.8;
      color: var(--clr-text-1);
      min-height: 120px;
      max-height: 200px;
      overflow-y: auto;
    }

    .log-line { display: block; }
    .log-line .ts   { color: var(--clr-text-2); }
    .log-line .ok   { color: var(--clr-accent); }
    .log-line .info { color: #60a5fa; }
    .log-line .warn { color: var(--clr-warn); }

    /* ─── Footer ────────────────────────────────────────────────────────── */
    footer {
      position: relative;
      z-index: 1;
      border-top: 1px solid var(--clr-border);
      padding: 1rem 1.5rem;
      text-align: center;
      font-size: 0.72rem;
      font-family: var(--font-mono);
      color: var(--clr-text-2);
    }

    /* ─── Responsive ────────────────────────────────────────────────────── */
    @media (max-width: 600px) {
      .navbar__subtitle { display: none; }
      .kpi-card__value  { font-size: 1.4rem; }
    }
  </style>
</head>

<body>

  <!-- Navigation -->
  <nav class="navbar" role="navigation" aria-label="Main navigation">
    <div class="navbar__brand">
      <div class="navbar__icon" aria-hidden="true">&#9741;</div>
      <div>
        <div class="navbar__title">OPERATOR NODE</div>
        <div class="navbar__subtitle">192.168.4.1 &mdash; SoftAP Mode</div>
      </div>
    </div>
    <div class="navbar__status" aria-live="polite">
      <span class="status-dot" role="img" aria-label="Online"></span>
      <span id="live-clock">--:--:--</span>
    </div>
  </nav>

  <!-- Dashboard Body -->
  <main id="dashboard" role="main">

    <!-- KPI Row -->
    <p class="section-label">&#9632; System Telemetry</p>
    <div class="kpi-grid" role="region" aria-label="Key performance indicators">

      <div class="kpi-card" id="card-uptime">
        <div class="kpi-card__label">Node Uptime</div>
        <div class="kpi-card__value accent" id="kpi-uptime">00:00:00</div>
        <div class="kpi-card__unit">hh:mm:ss since boot</div>
      </div>

      <div class="kpi-card" id="card-clients">
        <div class="kpi-card__label">Connected Clients</div>
        <div class="kpi-card__value" id="kpi-clients">—</div>
        <div class="kpi-card__unit">devices on AP</div>
      </div>

      <div class="kpi-card" id="card-channel">
        <div class="kpi-card__label">Wi-Fi Channel</div>
        <div class="kpi-card__value" id="kpi-channel">6</div>
        <div class="kpi-card__unit">802.11 b/g/n</div>
      </div>

      <div class="kpi-card" id="card-dns">
        <div class="kpi-card__label">DNS Intercepts</div>
        <div class="kpi-card__value warn" id="kpi-dns">0</div>
        <div class="kpi-card__unit">queries hijacked</div>
      </div>

    </div>

    <!-- Info Panels -->
    <p class="section-label">&#9632; Network Configuration</p>
    <div class="info-grid" role="region" aria-label="Network configuration panels">

      <div class="panel" id="panel-ap">
        <div class="panel__header">
          <span class="panel__icon" aria-hidden="true">&#9698;</span> Access Point
        </div>
        <div class="panel__body">
          <div class="info-row">
            <span class="info-row__key">SSID</span>
            <span class="info-row__val">Operator-Node-01</span>
          </div>
          <div class="info-row">
            <span class="info-row__key">Security</span>
            <span class="info-row__val"><span class="badge badge--green">WPA2-PSK</span></span>
          </div>
          <div class="info-row">
            <span class="info-row__key">IP Address</span>
            <span class="info-row__val">192.168.4.1</span>
          </div>
          <div class="info-row">
            <span class="info-row__key">Subnet Mask</span>
            <span class="info-row__val">255.255.255.0</span>
          </div>
          <div class="info-row">
            <span class="info-row__key">Gateway</span>
            <span class="info-row__val">192.168.4.1</span>
          </div>
        </div>
      </div>

      <div class="panel" id="panel-services">
        <div class="panel__header">
          <span class="panel__icon" aria-hidden="true">&#9654;</span> Active Services
        </div>
        <div class="panel__body">
          <div class="info-row">
            <span class="info-row__key">DNS Server</span>
            <span class="info-row__val"><span class="badge badge--green">&#10003; UDP :53</span></span>
          </div>
          <div class="info-row">
            <span class="info-row__key">HTTP Server</span>
            <span class="info-row__val"><span class="badge badge--green">&#10003; TCP :80</span></span>
          </div>
          <div class="info-row">
            <span class="info-row__key">Captive Portal</span>
            <span class="info-row__val"><span class="badge badge--green">&#10003; ACTIVE</span></span>
          </div>
          <div class="info-row">
            <span class="info-row__key">Redirect Mode</span>
            <span class="info-row__val"><span class="badge badge--warn">302 FOUND</span></span>
          </div>
          <div class="info-row">
            <span class="info-row__key">Internet Access</span>
            <span class="info-row__val"><span class="badge badge--warn">&#10007; OFFLINE</span></span>
          </div>
        </div>
      </div>

    </div>

    <!-- Console Log -->
    <p class="section-label">&#9632; Event Log</p>
    <div class="console" id="console-panel" role="log" aria-label="System event log" aria-live="polite">
      <div class="console__header">
        <div class="console__dots" aria-hidden="true">
          <div class="console__dot"></div>
          <div class="console__dot"></div>
          <div class="console__dot"></div>
        </div>
        <span>operator-node-01 / event-stream</span>
      </div>
      <div class="console__body" id="console-output">
        <span class="log-line"><span class="ts">[BOOT] </span><span class="ok">SoftAP initialized — SSID: Operator-Node-01</span></span>
        <span class="log-line"><span class="ts">[BOOT] </span><span class="ok">IP address assigned: 192.168.4.1</span></span>
        <span class="log-line"><span class="ts">[BOOT] </span><span class="ok">DNS server started on UDP port 53</span></span>
        <span class="log-line"><span class="ts">[BOOT] </span><span class="ok">HTTP server started on TCP port 80</span></span>
        <span class="log-line"><span class="ts">[INFO] </span><span class="info">Captive portal active — awaiting client connections</span></span>
      </div>
    </div>

  </main>

  <!-- Footer -->
  <footer role="contentinfo">
    ESP32 Operator Node v1.0.0 &mdash; Standalone LAN Mode &mdash; No external connectivity
  </footer>

  <script>
    // ── Client-side Runtime ──────────────────────────────────────────────────
    "use strict";

    // Boot time for uptime calculation
    const BOOT_TS = Date.now();
    let dnsCounter = 0;

    // Format seconds → hh:mm:ss
    function fmtHMS(totalSeconds) {
      const h = Math.floor(totalSeconds / 3600);
      const m = Math.floor((totalSeconds % 3600) / 60);
      const s = totalSeconds % 60;
      return [h, m, s].map(v => String(v).padStart(2, '0')).join(':');
    }

    // Update live clock (HH:MM:SS local time)
    function tickClock() {
      const now  = new Date();
      const hms  = [now.getHours(), now.getMinutes(), now.getSeconds()]
                    .map(v => String(v).padStart(2, '0')).join(':');
      document.getElementById('live-clock').textContent = hms;
    }

    // Update uptime counter
    function tickUptime() {
      const elapsed = Math.floor((Date.now() - BOOT_TS) / 1000);
      document.getElementById('kpi-uptime').textContent = fmtHMS(elapsed);
    }

    // Simulate DNS intercept counter (illustrative — replace with real /api/stats endpoint)
    function tickDNS() {
      dnsCounter += Math.floor(Math.random() * 3);
      document.getElementById('kpi-dns').textContent = dnsCounter;
    }

    // Append log entry to console
    function logEvent(level, message) {
      const out  = document.getElementById('console-output');
      const line = document.createElement('span');
      line.className = 'log-line';
      const ts   = new Date().toLocaleTimeString();
      line.innerHTML =
        '<span class="ts">[' + ts + '] </span>' +
        '<span class="' + level + '">' + message + '</span>';
      out.appendChild(line);
      out.scrollTop = out.scrollHeight;
    }

    // ── Periodic Updates ─────────────────────────────────────────────────────
    setInterval(tickClock,  1000);
    setInterval(tickUptime, 1000);
    setInterval(tickDNS,    4000);

    // Initial render
    tickClock();
    tickUptime();

    // Simulated event log entries (illustrative)
    setTimeout(() => logEvent('info', 'Client device joined the network'), 3500);
    setTimeout(() => logEvent('warn', 'DNS query intercepted — redirecting to 192.168.4.1'), 7000);
    setTimeout(() => logEvent('ok',   'Captive portal page served successfully'), 8000);
  </script>

</body>
</html>
)=====";
