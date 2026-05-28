/**
 * useStats — Polls /api/stats every 5 seconds.
 *
 * In development (import.meta.env.DEV), returns realistic mock data so you
 * can work without a live ESP32. In production the real endpoint is called.
 *
 * Returns: { stats, loading, error, lastUpdated }
 */
import { useState, useEffect, useRef, useCallback } from 'react';

const POLL_INTERVAL_MS = 5000;

// ── Mock data for browser dev testing ────────────────────────────────────────
const MOCK_BASE = {
  ssid:          'Operator-Node-01',
  ip:            '192.168.4.1',
  channel:       6,
  mac:           'AA:BB:CC:DD:EE:FF',
  security:      'WPA2-PSK',
  subnet:        '255.255.255.0',
  gateway:       '192.168.4.1',
  total_heap:    327680,
};

function buildMockStats(bootTs) {
  const now = Date.now();
  const uptimeMs = now - bootTs;
  // Simulate heap slowly decreasing then recovering (memory churn)
  const heap_used = Math.floor(80000 + 30000 * Math.sin(uptimeMs / 30000));
  return {
    ...MOCK_BASE,
    uptime_ms:      uptimeMs,
    free_heap:      MOCK_BASE.total_heap - heap_used,
    clients:        Math.floor(1 + Math.random() * 2),       // 1-2 clients
    dns_intercepts: Math.floor(uptimeMs / 4000),             // ~1 per 4s
    cpu_freq_mhz:   240,
  };
}

// ── Hook ──────────────────────────────────────────────────────────────────────
export function useStats() {
  const [stats, setStats]             = useState(null);
  const [loading, setLoading]         = useState(true);
  const [error, setError]             = useState(null);
  const [lastUpdated, setLastUpdated] = useState(null);
  const bootTsRef                     = useRef(Date.now());
  const timerRef                      = useRef(null);

  const fetchStats = useCallback(async () => {
    try {
      if (import.meta.env.DEV) {
        // Mock path — no network call
        await new Promise(r => setTimeout(r, 120)); // simulate latency
        setStats(buildMockStats(bootTsRef.current));
      } else {
        const res = await fetch('/api/stats', {
          headers: { 'Accept': 'application/json' },
          signal: AbortSignal.timeout(4000),
        });
        if (!res.ok) throw new Error(`HTTP ${res.status}`);
        const data = await res.json();
        setStats(data);
      }
      setError(null);
      setLastUpdated(new Date());
    } catch (err) {
      setError(err.message ?? 'Failed to reach ESP32');
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => {
    fetchStats();
    timerRef.current = setInterval(fetchStats, POLL_INTERVAL_MS);
    return () => clearInterval(timerRef.current);
  }, [fetchStats]);

  return { stats, loading, error, lastUpdated };
}
