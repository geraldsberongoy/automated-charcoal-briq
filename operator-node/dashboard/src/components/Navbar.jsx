// Navbar.jsx — Top navigation bar with live clock, API health badge
import { useClock } from "../hooks/useClock";

export default function Navbar({ error, lastUpdated, ip }) {
  const time = useClock();

  let apiBadgeClass = "fetching";
  let apiBadgeText = "CONNECTING";
  if (lastUpdated && !error) {
    apiBadgeClass = "ok";
    apiBadgeText = "LIVE";
  }
  if (error) {
    apiBadgeClass = "err";
    apiBadgeText = "OFFLINE";
  }

  return (
    <nav className="navbar" role="navigation" aria-label="Main navigation">
      <div className="navbar__brand">
        <div className="navbar__icon" aria-hidden="true">
          ⬡
        </div>
        <div>
          <div className="navbar__title">Operator Node from React</div>
          <div className="navbar__subtitle">
            {ip ?? "192.168.4.1"} — SoftAP Mode
          </div>
        </div>
      </div>

      <div className="navbar__right">
        <span className={`navbar__api-badge ${apiBadgeClass}`}>
          /api/stats · {apiBadgeText}
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
