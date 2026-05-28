// KpiCard.jsx — Single metric card with optional colour variant + skeleton state
export default function KpiCard({ id, icon, label, value, unit, variant = '', loading = false }) {
  return (
    <div className="kpi-card" id={id}>
      <div className="kpi-card__accent-bar" />
      {icon && <span className="kpi-card__icon" aria-hidden="true">{icon}</span>}
      <div className="kpi-card__label">{label}</div>

      {loading
        ? <div className="kpi-card__skeleton" aria-busy="true" />
        : <div className={`kpi-card__value ${variant}`}>{value ?? '—'}</div>
      }

      <div className="kpi-card__unit">{unit}</div>
    </div>
  );
}
