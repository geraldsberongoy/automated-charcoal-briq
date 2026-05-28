import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [react()],

  // Base URL — the ESP32 serves assets from the root
  base: '/',

  build: {
    // Output goes straight into the PlatformIO LittleFS data/ partition root
    outDir: '../data',
    emptyOutDir: true,

    // Keep chunks reasonable for ESP32 flash constraints
    rollupOptions: {
      output: {
        // Single JS + CSS bundle — simpler for the ESP32 static file server
        manualChunks: undefined,
      },
    },
  },

  server: {
    port: 5173,
    // In dev mode, proxy /api/* to the mock endpoint (or real ESP32 IP)
    proxy: {
      '/api': {
        // Change to your ESP32's IP when testing against real hardware:
        // target: 'http://192.168.4.1',
        target: 'http://localhost:5173',
        changeOrigin: true,
        // No-op in dev — mock data is returned by the React app itself
        bypass(req) {
          if (req.url.startsWith('/api')) return req.url;
        },
      },
    },
  },
});
