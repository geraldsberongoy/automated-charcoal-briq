import { useState, useEffect, useRef } from 'react';

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
  const heap_used = Math.floor(80000 + 30000 * Math.sin(uptimeMs / 30000));
  return {
    ...MOCK_BASE,
    uptime_ms:      uptimeMs,
    free_heap:      MOCK_BASE.total_heap - heap_used,
    clients:        Math.floor(1 + Math.random() * 2),
    dns_intercepts: Math.floor(uptimeMs / 1000),             // ~1 per sec in real-time mode
    cpu_freq_mhz:   240,
  };
}

export function useStats() {
  const [stats, setStats]                     = useState(null);
  const [loading, setLoading]                 = useState(true);
  const [error, setError]                     = useState(null);
  const [lastUpdated, setLastUpdated]         = useState(null);
  const [connectionType, setConnectionType]   = useState('DISCONNECTED');
  const wsRef                                 = useRef(null);
  const reconnectTimerRef                     = useRef(null);
  const mockTimerRef                          = useRef(null);
  const bootTsRef                             = useRef(Date.now());

  useEffect(() => {
    if (import.meta.env.DEV) {
      // ── Dev Mode: Simulate Real-time WebSocket Updates ──────────────────────
      setConnectionType('MOCK');
      setLoading(false);
      mockTimerRef.current = setInterval(() => {
        setStats(buildMockStats(bootTsRef.current));
        setLastUpdated(new Date());
      }, 1000); // 1-second interval to simulate WebSocket telemetry speed

      return () => {
        if (mockTimerRef.current) clearInterval(mockTimerRef.current);
      };
    }

    // ── Production Mode: Establish WebSocket Telemetry ────────────────────────
    let reconnectDelay = 1000;

    function connectWS() {
      const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
      const wsUrl = `${protocol}//${window.location.host}/ws`;
      
      console.log(`[WS] Connecting to ${wsUrl}...`);
      setConnectionType('CONNECTING');
      
      const ws = new WebSocket(wsUrl);
      wsRef.current = ws;

      ws.onopen = () => {
        console.log('[WS] Connected successfully.');
        setConnectionType('WS');
        setLoading(false);
        setError(null);
        reconnectDelay = 1000; // Reset reconnect delay on success
      };

      ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          setStats(data);
          setLastUpdated(new Date());
        } catch (err) {
          console.error('[WS] Parse error:', err);
        }
      };

      ws.onclose = (event) => {
        console.log(`[WS] Connection closed (code: ${event.code}). Attempting reconnect...`);
        setConnectionType('OFFLINE');
        setError('Connection lost. Reconnecting...');
        
        // Auto-reconnect with linear/exponential backoff (up to 5s max delay)
        reconnectTimerRef.current = setTimeout(() => {
          reconnectDelay = Math.min(5000, reconnectDelay + 1000);
          connectWS();
        }, reconnectDelay);
      };

      ws.onerror = (err) => {
        console.error('[WS] Socket error:', err);
        ws.close();
      };
    }

    connectWS();

    // Cleanup on unmount
    return () => {
      if (wsRef.current) {
        wsRef.current.onclose = null; // Prevent trigger of close callback on manual cleanup
        wsRef.current.close();
      }
      if (reconnectTimerRef.current) clearTimeout(reconnectTimerRef.current);
    };
  }, []);

  return { stats, loading, error, lastUpdated, connectionType };
}
