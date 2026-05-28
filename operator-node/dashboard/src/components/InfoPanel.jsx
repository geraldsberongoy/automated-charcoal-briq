// InfoPanel.jsx — Labelled info panel with row-based key/value layout
export default function InfoPanel({ id, icon, title, rows }) {
  return (
    <div className="panel" id={id}>
      <div className="panel__header">
        <span className="panel__icon" aria-hidden="true">{icon}</span>
        {title}
      </div>
      <div className="panel__body">
        {rows.map(({ key, value }) => (
          <div className="info-row" key={key}>
            <span className="info-row__key">{key}</span>
            <span className="info-row__val">{value}</span>
          </div>
        ))}
      </div>
    </div>
  );
}
