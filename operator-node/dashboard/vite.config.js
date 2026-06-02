import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import { VitePWA } from 'vite-plugin-pwa';

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [
    react(),
    VitePWA({
      registerType: 'autoUpdate',
      injectRegister: 'auto',
      manifestFilename: 'manifest.json',
      includeAssets: ['favicon.svg', 'apple-touch-icon.png', 'favicon-32x32.png'],
      manifest: {
        name: 'ESP32 Operator Node Dashboard',
        short_name: 'OperatorNode',
        description: 'ESP32 Operator Node Local Area Network Dashboard',
        theme_color: '#0a0c10',
        background_color: '#0a0c10',
        display: 'standalone',
        orientation: 'any',
        start_url: '/',
        scope: '/',
        icons: [
          {
            src: 'favicon-32x32.png',
            sizes: '32x32',
            type: 'image/png'
          },
          {
            src: 'pwa-192x192.png',
            sizes: '192x192',
            type: 'image/png'
          },
          {
            src: 'pwa-512x512.png',
            sizes: '512x512',
            type: 'image/png'
          },
          {
            src: 'pwa-512x512.png',
            sizes: '512x512',
            type: 'image/png',
            purpose: 'any maskable'
          }
        ]
      },
      workbox: {
        globPatterns: ['**/*.{js,css,html,ico,png,svg,woff2}'],
        navigateFallbackDenylist: [/^\/api/]
      }
    })
  ],

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

