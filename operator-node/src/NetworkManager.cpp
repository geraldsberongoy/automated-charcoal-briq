// ============================================================================
// NetworkManager.cpp — SoftAP, DNS Hijack & HTTP Captive Portal Implementation
//
// See include/NetworkManager.h for public API documentation.
// ============================================================================

#include "NetworkManager.h"
#include "WebAssets.h"        // PROGMEM dashboard HTML payload

// ── Constructor ──────────────────────────────────────────────────────────────

NetworkManager::NetworkManager()
  : _httpServer(OperatorConfig::HTTP_PORT),
    _dnsQueryCount(0)
{
  // Member objects are default-constructed here.
  // No hardware access — that is deferred to begin().
}

// ── Public: begin() ──────────────────────────────────────────────────────────

bool NetworkManager::begin() {
  // ── Step 0: Serial initialisation ────────────────────────────────────────
  // Guard against double-initialisation if the caller already started Serial.
  if (!Serial) {
    Serial.begin(OperatorConfig::BAUD_RATE);
    delay(200); // Small grace period for USB CDC enumeration
  }

  Serial.println();
  Serial.println(F("============================================================"));
  Serial.println(F("  ESP32 Operator Node — Boot Sequence Starting"));
  Serial.println(F("============================================================"));
  Serial.printf ("[BOOT] Firmware compiled: %s %s\r\n", __DATE__, __TIME__);
  Serial.printf ("[BOOT] SDK version: %s\r\n", ESP.getSdkVersion());
  Serial.printf ("[BOOT] Free heap at boot: %u bytes\r\n", ESP.getFreeHeap());

  // ── Step 1: Wi-Fi SoftAP ─────────────────────────────────────────────────
  if (!_initSoftAP()) {
    Serial.println(F("[ERROR] SoftAP initialisation FAILED — halting."));
    return false;
  }

  // ── Step 2: DNS Server ───────────────────────────────────────────────────
  _initDNS();

  // ── Step 3: HTTP Server ──────────────────────────────────────────────────
  _initHTTP();

  // ── Step 4: Print summary ────────────────────────────────────────────────
  _printNetworkInfo();

  Serial.println(F("[BOOT] All subsystems nominal. Entering main loop."));
  Serial.println(F("============================================================"));

  return true;
}

// ── Public: handle() ─────────────────────────────────────────────────────────

void NetworkManager::handle() {
  // Pump the DNS server — processes one UDP packet per call (non-blocking)
  _dnsServer.processNextRequest();

  // Pump the HTTP server — handles one pending TCP client per call (non-blocking)
  _httpServer.handleClient();
}

// ── Public: getClientCount() ─────────────────────────────────────────────────

uint8_t NetworkManager::getClientCount() const {
  // WiFi.softAPgetStationNum() is a lightweight call; safe to invoke each loop.
  return static_cast<uint8_t>(WiFi.softAPgetStationNum());
}

// ── Private: _initSoftAP() ───────────────────────────────────────────────────

bool NetworkManager::_initSoftAP() {
  Serial.println(F("[WIFI] Configuring Wi-Fi mode: AP_ONLY"));
  WiFi.mode(WIFI_AP);

  // Set the static IP, gateway, and subnet BEFORE calling softAP().
  // This is the correct LwIP ordering on Arduino-ESP32.
  IPAddress apIP, apGateway, apSubnet;

  if (!apIP.fromString(OperatorConfig::AP_IP) ||
      !apGateway.fromString(OperatorConfig::AP_GATEWAY) ||
      !apSubnet.fromString(OperatorConfig::AP_SUBNET)) {
    Serial.println(F("[ERROR] Invalid IP configuration string — check OperatorConfig."));
    return false;
  }

  Serial.printf("[WIFI] Configuring static IP: %s / %s via %s\r\n",
                OperatorConfig::AP_IP,
                OperatorConfig::AP_SUBNET,
                OperatorConfig::AP_GATEWAY);

  // softAPConfig() must be called before softAP() to take effect
  if (!WiFi.softAPConfig(apIP, apGateway, apSubnet)) {
    Serial.println(F("[ERROR] softAPConfig() failed — LwIP stack error."));
    return false;
  }

  Serial.printf("[WIFI] Starting SoftAP: SSID=\"%s\" | Channel=%d | MaxConn=%d | Hidden=%s\r\n",
                OperatorConfig::AP_SSID,
                OperatorConfig::AP_CHANNEL,
                OperatorConfig::AP_MAX_CONN,
                OperatorConfig::AP_HIDDEN ? "YES" : "NO");

  // softAP(ssid, password, channel, ssid_hidden, max_connection)
  // WPA2-PSK is enforced automatically when a password is provided.
  bool apStarted = WiFi.softAP(
    OperatorConfig::AP_SSID,
    OperatorConfig::AP_PASS,
    OperatorConfig::AP_CHANNEL,
    OperatorConfig::AP_HIDDEN,
    OperatorConfig::AP_MAX_CONN
  );

  if (!apStarted) {
    Serial.println(F("[ERROR] WiFi.softAP() returned false."));
    return false;
  }

  // Brief delay to allow the LwIP DHCP/AP stack to settle
  delay(500);

  Serial.printf("[WIFI] SoftAP active — IP: %s | MAC: %s\r\n",
                WiFi.softAPIP().toString().c_str(),
                WiFi.softAPmacAddress().c_str());

  return true;
}

// ── Private: _initDNS() ──────────────────────────────────────────────────────

void NetworkManager::_initDNS() {
  Serial.printf("[DNS ] Starting DNS server on UDP port %d\r\n",
                OperatorConfig::DNS_PORT);

  // TTL (2nd arg) = 60 seconds. The wildcard domain "*" captures ALL queries.
  // All DNS responses will resolve to the gateway IP, forcing clients to the portal.
  _dnsServer.setTTL(60);
  _dnsServer.setErrorReplyCode(DNSReplyCode::NoError);

  IPAddress dnsRedirectIP;
  dnsRedirectIP.fromString(OperatorConfig::AP_IP);

  // start(port, domain, ip_to_resolve_to)
  // The wildcard "*" ensures every DNS lookup returns our gateway IP.
  _dnsServer.start(OperatorConfig::DNS_PORT, "*", dnsRedirectIP);

  Serial.printf("[DNS ] Wildcard hijack active — all queries resolve to %s\r\n",
                OperatorConfig::AP_IP);
}

// ── Private: _initHTTP() ─────────────────────────────────────────────────────

void NetworkManager::_initHTTP() {
  Serial.printf("[HTTP] Registering HTTP routes on TCP port %d\r\n",
                OperatorConfig::HTTP_PORT);

  // ── Route: GET / ─────────────────────────────────────────────────────────
  // Serves the full PROGMEM dashboard HTML. This is the landing page.
  _httpServer.on("/", HTTP_GET, [this]() {
    Serial.printf("[HTTP] GET / from %s — serving dashboard\r\n",
                  _httpServer.client().remoteIP().toString().c_str());
    _handleRoot();
  });

  // ── Route: GET /generate_204 ─────────────────────────────────────────────
  // Android captive-portal probe. Normally expects HTTP 204 No Content.
  // Returning 302 forces Android to show the captive portal login notification.
  _httpServer.on("/generate_204", HTTP_GET, [this]() {
    Serial.printf("[HTTP] GET /generate_204 (Android probe) from %s — issuing 302\r\n",
                  _httpServer.client().remoteIP().toString().c_str());
    _handleRedirect();
  });

  // ── Route: GET /hotspot-detect.html ──────────────────────────────────────
  // Apple iOS/macOS captive-portal probe. Expects "Success" body.
  // 302 redirect triggers the Apple captive portal pop-up.
  _httpServer.on("/hotspot-detect.html", HTTP_GET, [this]() {
    Serial.printf("[HTTP] GET /hotspot-detect.html (Apple probe) from %s — issuing 302\r\n",
                  _httpServer.client().remoteIP().toString().c_str());
    _handleRedirect();
  });

  // ── Route: GET /connecttest.txt ──────────────────────────────────────────
  // Windows NCSI (Network Connectivity Status Indicator) probe.
  _httpServer.on("/connecttest.txt", HTTP_GET, [this]() {
    Serial.printf("[HTTP] GET /connecttest.txt (Windows NCSI probe) from %s — issuing 302\r\n",
                  _httpServer.client().remoteIP().toString().c_str());
    _handleRedirect();
  });

  // ── Route: GET /ncsi.txt ─────────────────────────────────────────────────
  // Alternate Windows NCSI probe path.
  _httpServer.on("/ncsi.txt", HTTP_GET, [this]() {
    Serial.printf("[HTTP] GET /ncsi.txt (Windows NCSI probe) from %s — issuing 302\r\n",
                  _httpServer.client().remoteIP().toString().c_str());
    _handleRedirect();
  });

  // ── Catch-all: any unmatched URI ─────────────────────────────────────────
  // This is the critical captive-portal hook: any page the device tries to
  // load (e.g., http://example.com/anything) is intercepted and redirected.
  _httpServer.onNotFound([this]() {
    Serial.printf("[HTTP] 404 catch-all for URI=%s from %s — issuing 302\r\n",
                  _httpServer.uri().c_str(),
                  _httpServer.client().remoteIP().toString().c_str());
    _handleRedirect();
  });

  _httpServer.begin();
  Serial.println(F("[HTTP] WebServer started. Routes: /, /generate_204, /hotspot-detect.html, /connecttest.txt, /ncsi.txt, *"));
}

// ── Private: _handleRoot() ───────────────────────────────────────────────────

void NetworkManager::_handleRoot() {
  // send_P() streams directly from PROGMEM — no heap copy required.
  _httpServer.send_P(
    200,
    "text/html; charset=utf-8",
    OPERATOR_DASHBOARD_HTML
  );
}

// ── Private: _handleRedirect() ───────────────────────────────────────────────

void NetworkManager::_handleRedirect() {
  // Build the destination URL from our configured gateway IP.
  // Using a String here is acceptable — this path is infrequent (one redirect per client).
  String redirectURL = "http://";
  redirectURL += OperatorConfig::AP_IP;
  redirectURL += "/";

  // HTTP 302 Found — temporary redirect.
  // The "Location" header is what triggers the OS captive portal flow.
  _httpServer.sendHeader("Location", redirectURL, true);
  _httpServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  _httpServer.send(302, "text/plain", "Redirecting to Operator Dashboard...");
}

// ── Private: _printNetworkInfo() ─────────────────────────────────────────────

void NetworkManager::_printNetworkInfo() const {
  Serial.println(F("[INFO] ─── Network Configuration Summary ───────────────────"));
  Serial.printf ("[INFO]  SSID          : %s\r\n",       OperatorConfig::AP_SSID);
  Serial.printf ("[INFO]  Security      : WPA2-PSK\r\n");
  Serial.printf ("[INFO]  IP Address    : %s\r\n",       WiFi.softAPIP().toString().c_str());
  Serial.printf ("[INFO]  MAC Address   : %s\r\n",       WiFi.softAPmacAddress().c_str());
  Serial.printf ("[INFO]  Subnet Mask   : %s\r\n",       OperatorConfig::AP_SUBNET);
  Serial.printf ("[INFO]  Gateway       : %s\r\n",       OperatorConfig::AP_GATEWAY);
  Serial.printf ("[INFO]  Wi-Fi Channel : %d\r\n",       OperatorConfig::AP_CHANNEL);
  Serial.printf ("[INFO]  Max Clients   : %d\r\n",       OperatorConfig::AP_MAX_CONN);
  Serial.printf ("[INFO]  DNS Port      : %d (UDP)\r\n", OperatorConfig::DNS_PORT);
  Serial.printf ("[INFO]  HTTP Port     : %d (TCP)\r\n", OperatorConfig::HTTP_PORT);
  Serial.println(F("[INFO] ────────────────────────────────────────────────────────"));
}
