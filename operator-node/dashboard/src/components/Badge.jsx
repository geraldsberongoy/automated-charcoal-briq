// Badge.jsx — Colour-coded status badge
export default function Badge({ children, variant = 'green' }) {
  return (
    <span className={`badge badge--${variant}`}>
      {children}
    </span>
  );
}
