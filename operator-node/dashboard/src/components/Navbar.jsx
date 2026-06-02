// Navbar.jsx — Top navigation bar with live clock, API health badge
import { useClock } from "../hooks/useClock";

export default function Navbar({ error, lastUpdated, ip, connectionType, isAdmin, onToggleAdmin }) {
  const time = useClock();

  let apiBadgeClass = "fetching";
  let apiBadgeText = "CONNECTING";
  let label = "WS TELEMETRY";

  if (connectionType === 'WS') {
    apiBadgeClass = "ok";
    apiBadgeText = "LIVE";
  } else if (connectionType === 'MOCK') {
    apiBadgeClass = "ok";
    apiBadgeText = "DEV MODE";
    label = "MOCK STREAM";
  } else if (connectionType === 'CONNECTING') {
    apiBadgeClass = "fetching";
    apiBadgeText = "CONNECTING";
  } else if (connectionType === 'OFFLINE') {
    apiBadgeClass = "err";
    apiBadgeText = "DISCONNECTED";
  } else {
    // Fallback/Legacy HTTP polling
    label = "/api/stats";
    if (lastUpdated && !error) {
      apiBadgeClass = "ok";
      apiBadgeText = "LIVE";
    }
    if (error) {
      apiBadgeClass = "err";
      apiBadgeText = "OFFLINE";
    }
  }

  return (
    <nav className="navbar" role="navigation" aria-label="Main navigation">
      <div className="navbar__brand">
        <div className="navbar__icon" aria-hidden="true">
          ⬡
        </div>
        <div>
          <div className="navbar__title">
            Operator Node<span className="navbar__title-suffix"> from React</span>
          </div>
          <div className="navbar__subtitle">
            {ip ?? "192.168.4.1"} — SoftAP Mode
          </div>
        </div>
      </div>

      <div className="navbar__right">
        <button
          className={`admin-toggle-btn ${isAdmin ? 'admin-active' : ''}`}
          onClick={onToggleAdmin}
          title={isAdmin ? "Click to lock Admin Mode" : "Click to unlock Admin Mode"}
          style={{
            background: 'none',
            color: isAdmin ? 'var(--clr-accent)' : 'var(--clr-text-2)',
            cursor: 'pointer',
            fontSize: '0.72rem',
            display: 'flex',
            alignItems: 'center',
            gap: '4px',
            fontFamily: 'var(--font-mono)',
            padding: '3px 8px',
            borderRadius: 'var(--radius-sm)',
            border: '1px solid',
            borderColor: isAdmin ? 'rgba(0,212,170,0.3)' : 'var(--clr-border)',
            backgroundColor: isAdmin ? 'rgba(0,212,170,0.05)' : 'transparent',
            outline: 'none',
            transition: 'all 0.15s ease',
          }}
        >
          {isAdmin ? '🔓 ADMIN' : '🔒 OPERATOR'}
        </button>

        <span className={`navbar__api-badge ${apiBadgeClass}`}>
          {label} · {apiBadgeText}
        </span>

        <div className="navbar__status" aria-live="polite">
          <span
            className={`status-dot ${error ? "err" : ""}`}
            role="img"
            aria-label={error ? "Offline" : "Online"}
          />
          <span id="live-clock">{time}</span>
        </div>
      </div>
    </nav>
  );
}
