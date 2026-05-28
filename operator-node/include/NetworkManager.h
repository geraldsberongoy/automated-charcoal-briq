#pragma once
// ============================================================================
// NetworkManager.h — SoftAP, DNS Hijack & HTTP Captive Portal Interface
//
// This module owns the complete LwIP-backed networking stack:
//   1. Wi-Fi SoftAP configuration (WPA2-PSK, static IP, subnet, gateway)
//   2. DNS server on UDP port 53 (wildcard → gateway IP)
//   3. HTTP server on TCP port 80 (dashboard page + 302 captive-portal redirects)
//
// Usage (in main.cpp):
//   NetworkManager nm;
//   nm.begin();          // call once in setup()
//   nm.handle();         // call every loop iteration
//
// Dependency injection: all state is encapsulated inside the class.
// No external global variables are created or required.
// ============================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

// ── Compile-time configuration constants ────────────────────────────────────
// (Centralised here so they never leak as unnamed literals into .cpp files)

namespace OperatorConfig {
  // SoftAP credentials
  constexpr char AP_SSID[]     = "Operator-Node-01";
  constexpr char AP_PASS[]     = "OperatorPassword";

  // Static IP addressing — 192.168.4.0/24
  constexpr char AP_IP[]       = "192.168.4.1";
  constexpr char AP_GATEWAY[]  = "192.168.4.1";
  constexpr char AP_SUBNET[]   = "255.255.255.0";

  // Wi-Fi channel (1–13). Channel 6 avoids overlap with 1 & 11.
  constexpr uint8_t AP_CHANNEL = 6;

  // Maximum simultaneous SoftAP client connections
  constexpr uint8_t AP_MAX_CONN = 8;

  // Hide SSID from broadcast? (false = visible, true = hidden)
  constexpr bool AP_HIDDEN = false;

  // DNS server listens on UDP port 53
  constexpr uint8_t DNS_PORT = 53;

  // HTTP server port
  constexpr uint16_t HTTP_PORT = 80;

  // Serial baud rate (must match platformio.ini monitor_speed)
  constexpr uint32_t BAUD_RATE = 115200;
}


// ── NetworkManager Class ─────────────────────────────────────────────────────

class NetworkManager {
public:
  // ── Constructor ─────────────────────────────────────────────────────────
  // Initialises member objects but does NOT touch hardware yet.
  // Hardware is initialised in begin().
  NetworkManager();

  // ── Public Interface ─────────────────────────────────────────────────────

  /**
   * @brief  Initialise all networking subsystems.
   *
   * Must be called once during setup(). Performs:
   *   1. Serial initialisation (if not already started)
   *   2. Wi-Fi SoftAP bring-up with static IP configuration
   *   3. DNS server start (wildcard hijack)
   *   4. HTTP route registration + server start
   *
   * @return true if the SoftAP was successfully started, false otherwise.
   */
  bool begin();

  /**
   * @brief  Pump all network service queues.
   *
   * Must be called every loop() iteration. Delegates to:
   *   • DNSServer::processNextRequest()
   *   • WebServer::handleClient()
   */
  void handle();

  /**
   * @brief  Returns the number of currently associated SoftAP clients.
   */
  uint8_t getClientCount() const;

private:
  // ── Private Members ──────────────────────────────────────────────────────

  DNSServer  _dnsServer;   // Owns the UDP:53 listening socket
  WebServer  _httpServer;  // Owns the TCP:80 listening socket

  // Incremented on every DNS query processed (diagnostic counter)
  uint32_t   _dnsQueryCount;

  // ── Private Helpers ──────────────────────────────────────────────────────

  /**
   * @brief  Configure the SoftAP and assign the static IP / subnet / gateway.
   * @return true on success.
   */
  bool _initSoftAP();

  /**
   * @brief  Start the DNS server and configure the wildcard redirect rule.
   */
  void _initDNS();

  /**
   * @brief  Register all HTTP routes and start the web server.
   *         Routes:
   *           GET  /           → serve dashboard HTML (200 OK)
   *           GET  /generate_204   → 302 redirect (Android captive-portal probe)
   *           GET  /hotspot-detect.html → 302 redirect (Apple captive-portal probe)
   *           ALL  *           → 302 redirect to dashboard (catch-all)
   */
  void _initHTTP();

  // ── HTTP Route Handlers (registered via lambda in _initHTTP) ─────────────

  /**
   * @brief  Serve the PROGMEM dashboard page.
   */
  void _handleRoot();

  /**
   * @brief  Emit a 302 redirect to the dashboard gateway IP.
   *         Called for all non-root URIs to trigger the OS captive portal flow.
   */
  void _handleRedirect();

  /**
   * @brief  Pretty-print the current IP configuration to Serial.
   */
  void _printNetworkInfo() const;
};
