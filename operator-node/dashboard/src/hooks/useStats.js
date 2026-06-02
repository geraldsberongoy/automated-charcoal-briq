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
  
  // 60-second operational cycle simulation
  const cycleDuration = 60000;
  const elapsed = uptimeMs % cycleDuration;
  
  let activeState = 1;
  let safetySwitch = 1; // 1 = closed/safe, 0 = open/breached
  
  let flow = 0.0;
  let force = 0.0;
  let temp = 27.5; // ambient base
  let rh = 62.0;    // ambient base
  let grinderA = 0.0;
  
  if (elapsed < 5000) {
    // Stage 1: Initialization
    activeState = 1;
  } else if (elapsed < 18000) {
    // Stage 2: Grinding & Soaking (13 seconds)
    activeState = 2;
    const p = (elapsed - 5000) / 13000;
    flow = (p * 3.0).toFixed(2); // filling up to 3 liters
    grinderA = (3.5 + Math.random() * 0.8).toFixed(1); // active grinding current
  } else if (elapsed < 30000) {
    // Stage 3: Combining & Mixing (12 seconds)
    activeState = 3;
  } else if (elapsed < 45000) {
    // Stage 4: Compression & Molding (15 seconds)
    activeState = 4;
    const stageElapsed = elapsed - 30000;
    
    // Simulate a brief casing breach / emergency halt between 34s and 37s (3 seconds)
    if (stageElapsed > 4000 && stageElapsed < 7000) {
      safetySwitch = 0;
      force = 850.0; // frozen during breach
    } else {
      safetySwitch = 1;
      // adjust interpolation to skip breach duration
      let p = stageElapsed / 15000;
      if (stageElapsed >= 7000) {
        p = (stageElapsed - 3000) / 12000;
      }
      force = Math.min(2000, Math.floor(p * 2200));
    }
  } else {
    // Stage 5: Thermal Curing (15 seconds)
    activeState = 5;
    const p = (elapsed - 45000) / 15000;
    temp = (27.5 + p * 22.5).toFixed(1); // heats up to 50C
    rh = Math.max(15.0, (62.0 - p * 47.0)).toFixed(1); // humidity drops to 15%
  }

  // Simulate slowly draining/charging battery
  const batteryV = (13.1 + 0.3 * Math.sin(uptimeMs / 120000)).toFixed(1);
  const solarA = (activeState === 5 ? 0.8 : (1.5 + Math.random() * 0.5)).toFixed(1); // higher charging when lamps are off

  const heap_used = Math.floor(80000 + 30000 * Math.sin(uptimeMs / 30000));
  return {
    ...MOCK_BASE,
    uptime_ms:      uptimeMs,
    free_heap:      MOCK_BASE.total_heap - heap_used,
    clients:        Math.floor(2 + Math.random() * 1), // 2-3 active devices
    dns_intercepts: Math.floor(uptimeMs / 3000),       // ~1 query every 3s
    cpu_freq_mhz:   240,
    active_state:   activeState,
    safety_switch:  safetySwitch,
    sensors: {
      volumetric_flow_l:  parseFloat(flow),
      compaction_force_n: parseFloat(force),
      curing_temp_c:      parseFloat(temp),
      relative_humidity:  parseFloat(rh),
      grinder_current_a:  parseFloat(grinderA)
    },
    power: {
      battery_voltage_v:  parseFloat(batteryV),
      solar_amperage_a:   parseFloat(solarA)
    }
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
