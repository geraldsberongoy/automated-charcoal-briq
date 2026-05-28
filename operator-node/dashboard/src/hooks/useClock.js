/**
 * useClock — Updates every second, returns current time as HH:MM:SS string.
 */
import { useState, useEffect } from 'react';

function fmt(n) { return String(n).padStart(2, '0'); }

export function useClock() {
  const [time, setTime] = useState('--:--:--');

  useEffect(() => {
    const tick = () => {
      const d = new Date();
      setTime(`${fmt(d.getHours())}:${fmt(d.getMinutes())}:${fmt(d.getSeconds())}`);
    };
    tick();
    const id = setInterval(tick, 1000);
    return () => clearInterval(id);
  }, []);

  return time;
}
