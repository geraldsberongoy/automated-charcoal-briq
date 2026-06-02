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

function updateMockTelemetry(prev, bootTs) {
  const now = Date.now();
  const uptimeMs = now - bootTs;

  if (!prev) {
    return {
      ...MOCK_BASE,
      uptime_ms: 0,
      free_heap: 247680,
      clients: 2,
      dns_intercepts: 0,
      cpu_freq_mhz: 240,
      active_state: 1,
      safety_switch: 1,
      sensors: {
        volumetric_flow_l: 0.0,
        compaction_force_n: 0.0,
        curing_temp_c: 27.5,
        relative_humidity: 62.0,
        grinder_current_a: 0.0
      },
      power: {
        battery_voltage_v: 13.1,
        solar_amperage_a: 1.8
      }
    };
  }

  const next = { ...prev };
  next.uptime_ms = uptimeMs;
  next.dns_intercepts = Math.floor(uptimeMs / 3000);

  // Fluctuating power levels
  next.power.battery_voltage_v = parseFloat((12.9 + 0.3 * Math.sin(uptimeMs / 60000)).toFixed(1));
  next.power.solar_amperage_a = parseFloat((1.5 + 0.5 * Math.random()).toFixed(1));

  // Heap usage fluctuations
  const heap_used = Math.floor(80000 + 30000 * Math.sin(uptimeMs / 30000));
  next.free_heap = MOCK_BASE.total_heap - heap_used;

  // If safety switch is tripped, freeze telemetry
  if (next.safety_switch === 0) {
    next.sensors.grinder_current_a = 0.0;
    return next;
  }

  // State-specific sensor simulation
  switch (next.active_state) {
    case 1:
      // Boot / Idle
      next.sensors.volumetric_flow_l = 0.0;
      next.sensors.compaction_force_n = 0.0;
      next.sensors.grinder_current_a = 0.0;
      next.sensors.curing_temp_c = 27.5;
      next.sensors.relative_humidity = 62.0;
      break;

    case 2:
      // Prep & Soaking (Grinder active, water flowing)
      if (next.sensors.volumetric_flow_l < 3.0) {
        next.sensors.volumetric_flow_l = parseFloat((next.sensors.volumetric_flow_l + 0.25).toFixed(2));
        next.sensors.grinder_current_a = parseFloat((3.8 + Math.random() * 0.6).toFixed(1));
      } else {
        next.sensors.volumetric_flow_l = 3.00;
        next.sensors.grinder_current_a = 0.0;
      }
      break;

    case 3:
      // Mixing
      next.sensors.volumetric_flow_l = 3.00;
      next.sensors.grinder_current_a = 0.0;
      break;

    case 4:
      // Compression
      if (next.sensors.compaction_force_n < 2000) {
        next.sensors.compaction_force_n = Math.min(2000, next.sensors.compaction_force_n + 150);
      }
      break;

    case 5:
      // Curing
      if (next.sensors.relative_humidity > 14.5) {
        next.sensors.relative_humidity = parseFloat(Math.max(14.5, next.sensors.relative_humidity - 3.5).toFixed(1));
        next.sensors.curing_temp_c = parseFloat((next.sensors.curing_temp_c + 1.8).toFixed(1));
      }
      break;

    default:
      break;
  }

  return next;
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
      setConnectionType('MOCK');
      setLoading(false);
      
      // Initialize with default state
      setStats(updateMockTelemetry(null, bootTsRef.current));
      setLastUpdated(new Date());

      mockTimerRef.current = setInterval(() => {
        setStats(prev => updateMockTelemetry(prev, bootTsRef.current));
        setLastUpdated(new Date());
      }, 1000);

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

  const sendCommand = (action, payload) => {
    if (import.meta.env.DEV || connectionType === 'MOCK') {
      console.log(`[MOCK CMD] Action: ${action}`, payload);
      setStats(prev => {
        if (!prev) return prev;
        const next = { ...prev };
        if (action === 'SET_STATE') {
          next.active_state = payload.state;
          // Reset sensors relevant to state transition
          if (payload.state === 1) {
            next.sensors.volumetric_flow_l = 0.0;
            next.sensors.compaction_force_n = 0.0;
            next.sensors.grinder_current_a = 0.0;
            next.sensors.curing_temp_c = 27.5;
            next.sensors.relative_humidity = 62.0;
          } else if (payload.state === 2) {
            next.sensors.volumetric_flow_l = 0.0;
            next.sensors.grinder_current_a = 4.2;
          } else if (payload.state === 3) {
            next.sensors.volumetric_flow_l = 3.0;
            next.sensors.grinder_current_a = 0.0;
          } else if (payload.state === 4) {
            next.sensors.compaction_force_n = 0.0;
          } else if (payload.state === 5) {
            next.sensors.relative_humidity = 62.0;
            next.sensors.curing_temp_c = 27.5;
          }
        } else if (action === 'RESET_FAULT') {
          next.safety_switch = 1;
        } else if (action === 'TRIP_SAFETY') {
          next.safety_switch = 0;
          next.sensors.compaction_force_n = 850.0;
        }
        return next;
      });
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
