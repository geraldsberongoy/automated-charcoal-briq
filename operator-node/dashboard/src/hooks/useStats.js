import { useState, useEffect, useRef } from 'react';
import { createMock, applyCommand, advance, toProcess } from './mockProcess';

const MOCK_BASE = {
  ssid:          'Operator-Node-01',
  ip:            '192.168.4.1',
  channel:       6,
  mac:           'AA:BB:CC:DD:EE:FF',
  security:      'WPA2-PSK',
  subnet:        '255.255.255.0',
  gateway:       '192.168.4.1',
  total_heap:    327680,
  cpu_freq_mhz:  240,
};

const MOCK_TICK_MS = 100;   // fine enough for the scaled-down pour bursts

// Full telemetry object in the same shape the firmware broadcasts
function buildMockStats(machine, bootTs, now) {
  const uptimeMs = now - bootTs;
  return {
    ...MOCK_BASE,
    uptime_ms: uptimeMs,
    free_heap: MOCK_BASE.total_heap - Math.floor(80000 + 30000 * Math.sin(uptimeMs / 30000)),
    clients: 2,
    dns_intercepts: Math.floor(uptimeMs / 3000),
    process: toProcess(machine, now),
    outputs: { ...machine.outputs },
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
  const machineRef                          = useRef(null);
  const lastTickRef                         = useRef(0);

  useEffect(() => {
    if (import.meta.env.DEV) {
      setConnectionType('MOCK');
      setLoading(false);

      const now = Date.now();
      machineRef.current = createMock(now);
      lastTickRef.current = now;
      setStats(buildMockStats(machineRef.current, bootTsRef.current, now));
      setLastUpdated(new Date());

      mockTimerRef.current = setInterval(() => {
        const t = Date.now();
        advance(machineRef.current, t, (t - lastTickRef.current) / 1000);
        lastTickRef.current = t;
        setStats(buildMockStats(machineRef.current, bootTsRef.current, t));
        setLastUpdated(new Date());
      }, MOCK_TICK_MS);

      return () => {
        if (mockTimerRef.current) clearInterval(mockTimerRef.current);
      };
    }

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
        reconnectDelay = 1000;
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

    return () => {
      if (wsRef.current) {
        wsRef.current.onclose = null;
        wsRef.current.close();
      }
      if (reconnectTimerRef.current) clearTimeout(reconnectTimerRef.current);
    };
  }, []);

  // Batch commands: SET_WATER {pct}, NEXT, BACK, START, STOP, RESET, DONE
  const sendCommand = (action, payload) => {
    if (import.meta.env.DEV || connectionType === 'MOCK') {
      console.log(`[MOCK CMD] Action: ${action}`, payload);
      if (machineRef.current) {
        const t = Date.now();
        applyCommand(machineRef.current, action, payload, t);
        setStats(buildMockStats(machineRef.current, bootTsRef.current, t));
      }
    } else {
      if (wsRef.current && wsRef.current.readyState === WebSocket.OPEN) {
        wsRef.current.send(JSON.stringify({ action, payload }));
        console.log(`[WS CMD] Sent: ${action}`, payload);
      } else {
        console.error('[WS CMD] WebSocket not connected.');
      }
    }
  };

  return { stats, loading, error, lastUpdated, connectionType, sendCommand };
}
