// HeapBar.jsx — Visual memory pressure bar shown inside KPI area
export default function HeapBar({ freeHeap, totalHeap }) {
  if (!totalHeap || !freeHeap) return null;

  const usedPct   = ((totalHeap - freeHeap) / totalHeap) * 100;
  const barClass  = usedPct > 80 ? 'crit' : usedPct > 60 ? 'warn' : '';
  const freeKB    = (freeHeap / 1024).toFixed(1);
  const totalKB   = (totalHeap / 1024).toFixed(0);

  return (
    <div className="heap-bar-wrap">
      <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: '0.7rem', fontFamily: 'var(--font-mono)', color: 'var(--clr-text-2)', marginBottom: '2px' }}>
        <span>FREE: {freeKB} KB</span>
        <span>{usedPct.toFixed(0)}% used</span>
      </div>
      <div className="heap-bar-track" title={`${freeKB} KB free of ${totalKB} KB total`}>
        <div
          className={`heap-bar-fill ${barClass}`}
          style={{ width: `${usedPct}%` }}
        />
      </div>
    </div>
  );
}
