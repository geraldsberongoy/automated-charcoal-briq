// EventLog.jsx — Auto-scrolling console log panel
import { useEffect, useRef } from 'react';

function fmt(d) {
  return `${String(d.getHours()).padStart(2,'0')}:${String(d.getMinutes()).padStart(2,'0')}:${String(d.getSeconds()).padStart(2,'0')}`;
}

export default function EventLog({ entries }) {
  const bodyRef = useRef(null);

  // Auto-scroll to bottom on new entries
  useEffect(() => {
    if (bodyRef.current) {
      bodyRef.current.scrollTop = bodyRef.current.scrollHeight;
    }
  }, [entries]);

  return (
    <div className="console" id="console-panel" role="log" aria-label="System event log" aria-live="polite">
      <div className="console__header">
        <div className="console__dots" aria-hidden="true">
          <div className="console__dot" />
          <div className="console__dot" />
          <div className="console__dot" />
        </div>
        <span>operator-node-01 / event-stream</span>
        <span style={{ color: 'var(--clr-text-2)' }}>{entries.length} entries</span>
      </div>
      <div className="console__body" id="console-output" ref={bodyRef}>
        {entries.map((e, i) => (
          <div className="log-line" key={i}>
            <span className="log-line__ts">[{fmt(e.time)}]</span>
            <span className={`log-line__level ${e.level}`}>[{e.level.toUpperCase()}]</span>
            <span className="log-line__msg">{e.msg}</span>
          </div>
        ))}
      </div>
    </div>
  );
}
