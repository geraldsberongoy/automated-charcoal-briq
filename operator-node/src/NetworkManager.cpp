// ============================================================================
// NetworkManager.cpp — SoftAP, DNS Hijack & HTTP Captive Portal Implementation
//
// See include/NetworkManager.h for public API documentation.
// ============================================================================

#include "NetworkManager.h"
#include "WebAssets.h"        // PROGMEM dashboard HTML payload (fallback)

// ── Constructor ──────────────────────────────────────────────────────────────

NetworkManager::NetworkManager()
  : _process(nullptr),
    _httpServer(OperatorConfig::HTTP_PORT),
    _ws("/ws"),
    _telemetryTaskHandle(nullptr),
    _dnsQueryCount(0),
    _littleFsMounted(false)
{
  // Member objects are default-constructed here.
  // No hardware access — that is deferred to begin().
}

// ── Public: begin() ──────────────────────────────────────────────────────────

bool NetworkManager::begin(BatchProcess &process) {
  _process = &process;

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

  // ── Step 3: LittleFS ─────────────────────────────────────────────────────
  _initLittleFS();

  // ── Step 4: HTTP Server ──────────────────────────────────────────────────
  _initHTTP();

  // ── Step 4.5: Spawn Real-time Telemetry task on Core 1 ───────────────────
  Serial.println(F("[RTOS] Spawning telemetry WebSocket task pinned to Core 1..."));
  BaseType_t taskCreated = xTaskCreatePinnedToCore(
    _telemetryTask,             // Task function
    "telemetry_ws",             // Task name
    4096,                       // Stack size in words (16KB)
    this,                       // Pass our class instance
    1,                          // Priority
    &_telemetryTaskHandle,      // Task handle
    1                           // CPU core (Core 1)
  );

  if (taskCreated != pdPASS) {
    Serial.println(F("[ERROR] Failed to create telemetry task — running on loop fallback."));
  }

  // ── Step 5: Print summary ────────────────────────────────────────────────
  _printNetworkInfo();

  Serial.println(F("[BOOT] All subsystems nominal. Entering main loop."));
  Serial.println(F("============================================================"));

  return true;
}

// ── Public: handle() ─────────────────────────────────────────────────────────

void NetworkManager::handle() {
  // Pump the DNS server — processes one UDP packet per call (non-blocking)
  _dnsServer.processNextRequest();

  // Clean up any stale/disconnected WebSocket clients
  _ws.cleanupClients();
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

// ── Private: _initLittleFS() ─────────────────────────────────────────────────

void NetworkManager::_initLittleFS() {
#if defined(LITTLEFS_ENABLED) && LITTLEFS_ENABLED
  Serial.println(F("[LFS ] Mounting LittleFS partition..."));

  if (!LittleFS.begin(true)) {  // true = format on failure
    Serial.println(F("[LFS ] Mount FAILED — falling back to PROGMEM WebAssets."));
    _littleFsMounted = false;
    return;
  }

  // Verify the React build artifact is present
  if (!LittleFS.exists("/index.html")) {
    Serial.println(F("[LFS ] /index.html not found — run 'npm run build' then 'Upload Filesystem Image'."));
    Serial.println(F("[LFS ] Falling back to PROGMEM WebAssets."));
    _littleFsMounted = false;
    return;
  }

  _littleFsMounted = true;

  // Log filesystem stats
  size_t total = LittleFS.totalBytes();
  size_t used  = LittleFS.usedBytes();
  Serial.printf("[LFS ] Mounted OK — %u KB used / %u KB total (%.1f%% full)\r\n",
                used  / 1024, total / 1024,
                total > 0 ? (float)used / total * 100.0f : 0.0f);
#else
  Serial.println(F("[LFS ] LITTLEFS_ENABLED not set — using PROGMEM WebAssets."));
  _littleFsMounted = false;
#endif
}

// ── Private: _initHTTP() ─────────────────────────────────────────────────────

void NetworkManager::_initHTTP() {
  Serial.printf("[HTTP] Registering HTTP routes on TCP port %d\r\n",
                OperatorConfig::HTTP_PORT);

  // ── WebSocket Handler ─────────────────────────────────────────────────────
  _ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, 
                     AwsEventType type, void *arg, uint8_t *data, size_t len) {
    _handleWsEvent(server, client, type, arg, data, len);
  });
  _httpServer.addHandler(&_ws);

  // ── Route: GET / ─────────────────────────────────────────────────────────
  _httpServer.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
    Serial.printf("[HTTP] GET / from %s\r\n",
                  request->client()->remoteIP().toString().c_str());
    _handleRoot(request);
  });

  // ── Route: GET /api/stats ────────────────────────────────────────────────
  _httpServer.on("/api/stats", HTTP_GET, [this](AsyncWebServerRequest *request) {
    _handleApiStats(request);
  });

  // ── Route: GET /generate_204 ─────────────────────────────────────────────
  _httpServer.on("/generate_204", HTTP_GET, [this](AsyncWebServerRequest *request) {
    Serial.printf("[HTTP] GET /generate_204 (Android probe) from %s — issuing 302\r\n",
                  request->client()->remoteIP().toString().c_str());
    _handleRedirect(request);
  });

  // ── Route: GET /hotspot-detect.html ──────────────────────────────────────
  _httpServer.on("/hotspot-detect.html", HTTP_GET, [this](AsyncWebServerRequest *request) {
    Serial.printf("[HTTP] GET /hotspot-detect.html (Apple probe) from %s — issuing 302\r\n",
                  request->client()->remoteIP().toString().c_str());
    _handleRedirect(request);
  });

  // ── Route: GET /connecttest.txt ──────────────────────────────────────────
  _httpServer.on("/connecttest.txt", HTTP_GET, [this](AsyncWebServerRequest *request) {
    Serial.printf("[HTTP] GET /connecttest.txt (Windows NCSI probe) from %s — issuing 302\r\n",
                  request->client()->remoteIP().toString().c_str());
    _handleRedirect(request);
  });

  // ── Route: GET /ncsi.txt ─────────────────────────────────────────────────
  _httpServer.on("/ncsi.txt", HTTP_GET, [this](AsyncWebServerRequest *request) {
    Serial.printf("[HTTP] GET /ncsi.txt (Windows NCSI probe) from %s — issuing 302\r\n",
                  request->client()->remoteIP().toString().c_str());
    _handleRedirect(request);
  });

  // ── Catch-all ────────────────────────────────────────────────────────────
  _httpServer.onNotFound([this](AsyncWebServerRequest *request) {
    const String uri = request->url();

#if defined(LITTLEFS_ENABLED) && LITTLEFS_ENABLED
    if (_littleFsMounted && LittleFS.exists(uri)) {
      _handleStaticFile(request);
      return;
    }
#endif

    Serial.printf("[HTTP] 404 catch-all for URI=%s from %s — issuing 302\r\n",
                  uri.c_str(),
                  request->client()->remoteIP().toString().c_str());
    _handleRedirect(request);
  });

  _httpServer.begin();
  Serial.println(F("[HTTP] WebServer started."));
  Serial.printf ("[HTTP] Routes: /, /api/stats, /generate_204, /hotspot-detect.html, /connecttest.txt, /ncsi.txt, *(LittleFS|302)\r\n");
}

// ── Private: _handleRoot() ───────────────────────────────────────────────────

void NetworkManager::_handleRoot(AsyncWebServerRequest *request) {
#if defined(LITTLEFS_ENABLED) && LITTLEFS_ENABLED
  if (_littleFsMounted && LittleFS.exists("/index.html")) {
    request->send(LittleFS, "/index.html", "text/html; charset=utf-8");
    return;
  }
#endif
  // Fallback: serve PROGMEM dashboard if LittleFS is unavailable
  Serial.println(F("[HTTP] Serving PROGMEM fallback dashboard."));
  request->send(200, "text/html; charset=utf-8", (const uint8_t*)OPERATOR_DASHBOARD_HTML, strlen_P(OPERATOR_DASHBOARD_HTML));
}

// ── Private: _handleStaticFile() ─────────────────────────────────────────────

void NetworkManager::_handleStaticFile(AsyncWebServerRequest *request) {
#if defined(LITTLEFS_ENABLED) && LITTLEFS_ENABLED
  const String path = request->url();

  String contentType = "application/octet-stream";
  if      (path.endsWith(".html")) contentType = "text/html; charset=utf-8";
  else if (path.endsWith(".js"))   contentType = "application/javascript";
  else if (path.endsWith(".css"))  contentType = "text/css";
  else if (path.endsWith(".svg"))  contentType = "image/svg+xml";
  else if (path.endsWith(".ico"))  contentType = "image/x-icon";
  else if (path.endsWith(".json")) contentType = "application/json";
  else if (path.endsWith(".png"))  contentType = "image/png";
  else if (path.endsWith(".woff2"))contentType = "font/woff2";

  AsyncWebServerResponse *response = request->beginResponse(LittleFS, path, contentType);
  if (path.startsWith("/assets/")) {
    response->addHeader("Cache-Control", "public, max-age=31536000, immutable");
  } else if (path == "/sw.js") {
    response->addHeader("Cache-Control", "no-store, no-cache, must-revalidate");
  }
  request->send(response);
#else
  request->send(404, "text/plain", "LittleFS not enabled");
#endif
}

// ── Private: _handleApiStats() ───────────────────────────────────────────────

void NetworkManager::_handleApiStats(AsyncWebServerRequest *request) {
  char buf[TELEMETRY_JSON_MAX];
  _buildTelemetryJson(buf, sizeof(buf));

  AsyncWebServerResponse *response = request->beginResponse(200, "application/json", buf);
  response->addHeader("Access-Control-Allow-Origin", "*");
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
}

// ── Private: _handleRedirect() ───────────────────────────────────────────────

void NetworkManager::_handleRedirect(AsyncWebServerRequest *request) {
  // Build the destination URL from our configured gateway IP.
  String redirectURL = "http://";
  redirectURL += OperatorConfig::AP_IP;
  redirectURL += "/";

  AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Redirecting to Operator Dashboard...");
  response->addHeader("Location", redirectURL);
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  request->send(response);
}

// ── Private: _handleWsEvent() ────────────────────────────────────────────────

void NetworkManager::_handleWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                                    AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("[WS  ] Client connected: ID=%u, IP=%s\r\n", 
                  client->id(), client->remoteIP().toString().c_str());
    
    // Provide an immediate telemetry update so the joining device gets instant status
    char buf[TELEMETRY_JSON_MAX];
    _buildTelemetryJson(buf, sizeof(buf));
    client->text(buf);
  } 
  else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("[WS  ] Client disconnected: ID=%u\r\n", client->id());
  }
  else if (type == WS_EVT_DATA) {
    // Only single-frame text messages — dashboard commands are tiny JSON
    AwsFrameInfo *info = static_cast<AwsFrameInfo*>(arg);
    if (!info->final || info->index != 0 || info->len != len || info->opcode != WS_TEXT) return;

    char msg[128];
    const size_t n = min(len, sizeof(msg) - 1);
    memcpy(msg, data, n);
    msg[n] = '\0';
    _handleWsCommand(msg);
  }
}

// ── Private: _handleWsCommand() ──────────────────────────────────────────────
// Payload shape: {"action":"<ACTION>","payload":{...}}
//   SET_WATER {"pct":80} | NEXT | BACK | START | STOP | RESET | DONE
//   TRIP_SAFETY is accepted as an alias for STOP.
// Commands are queued; the batch FSM applies them on the next loop tick and
// the regular 1 s broadcast carries the result.

void NetworkManager::_handleWsCommand(const char *msg) {
  struct Entry { const char *name; Cmd cmd; };
  static const Entry ACTIONS[] = {
    {"\"SET_WATER\"", Cmd::SET_WATER}, {"\"NEXT\"", Cmd::NEXT},
    {"\"BACK\"", Cmd::BACK},           {"\"START\"", Cmd::START},
    {"\"STOP\"", Cmd::STOP},           {"\"TRIP_SAFETY\"", Cmd::STOP},
    {"\"RESET\"", Cmd::RESET},         {"\"DONE\"", Cmd::DONE},
  };

  for (const Entry &e : ACTIONS) {
    if (!strstr(msg, e.name)) continue;

    uint8_t arg = 0;
    if (e.cmd == Cmd::SET_WATER) {
      const char *key   = strstr(msg, "\"pct\"");
      const char *colon = key ? strchr(key, ':') : nullptr;
      if (!colon) return;
      arg = static_cast<uint8_t>(constrain(atoi(colon + 1), 0, 255));
    }
    Serial.printf("[WS  ] CMD %s %u\r\n", e.name, arg);
    _process->post(e.cmd, arg);
    return;
  }

  Serial.printf("[WS  ] Unknown command: %s\r\n", msg);
}

// ── Private: _buildTelemetryJson() ───────────────────────────────────────────

void NetworkManager::_buildTelemetryJson(char *buf, size_t size) {
  const BatchSnapshot b = _process->snapshot();

  static const char *const DRIVE[] = {"OFF", "ON", "EXTEND", "RETRACT"};
  auto drive = [&b](Output o) { return DRIVE[static_cast<uint8_t>(b.outputs[static_cast<uint8_t>(o)])]; };

  snprintf(buf, size,
    "{"
      "\"uptime_ms\":%lu,"
      "\"free_heap\":%u,"
      "\"total_heap\":%u,"
      "\"clients\":%d,"
      "\"dns_intercepts\":%lu,"
      "\"ssid\":\"%s\","
      "\"ip\":\"%s\","
      "\"channel\":%d,"
      "\"mac\":\"%s\","
      "\"subnet\":\"%s\","
      "\"gateway\":\"%s\","
      "\"cpu_freq_mhz\":%u,"
      "\"security\":\"WPA2-PSK\","
      "\"process\":{"
        "\"step\":\"%s\","
        "\"run_index\":%u,"
        "\"stopped_during\":\"%s\","
        "\"water_pct\":%u,"
        "\"target_l\":%.2f,"
        "\"flow_l\":%.2f,"
        "\"force_n\":%.0f,"
        "\"peak_force_n\":%.0f,"
        "\"mixer_a\":%.1f,"
        "\"step_elapsed_ms\":%lu,"
        "\"step_duration_ms\":%lu,"
        "\"pulse\":%u,"
        "\"press_holding\":%s,"
        "\"reset_phase\":%u,"
        "\"batch_ms\":%lu"
      "},"
      "\"outputs\":{"
        "\"solenoid\":\"%s\","
        "\"mixer_motor\":\"%s\","
        "\"gate_actuator\":\"%s\","
        "\"mixer_erm\":\"%s\","
        "\"box_erm\":\"%s\","
        "\"press_actuator\":\"%s\""
      "},"
      "\"power\":{"
        "\"battery_voltage_v\":%.1f,"
        "\"solar_amperage_a\":%.1f"
      "}"
    "}",
    (unsigned long)millis(),
    (unsigned int)ESP.getFreeHeap(),
    (unsigned int)ESP.getHeapSize(),
    (int)WiFi.softAPgetStationNum(),
    (unsigned long)_dnsQueryCount,
    OperatorConfig::AP_SSID,
    WiFi.softAPIP().toString().c_str(),
    (int)OperatorConfig::AP_CHANNEL,
    WiFi.softAPmacAddress().c_str(),
    OperatorConfig::AP_SUBNET,
    OperatorConfig::AP_GATEWAY,
    (unsigned int)getCpuFrequencyMhz(),
    stepName(b.step),
    (unsigned)b.runIndex,
    stepName(b.stoppedDuring),
    (unsigned)b.waterPct,
    b.targetL, b.flowL, b.forceN, b.peakForceN, b.mixerA,
    (unsigned long)b.stepElapsedMs,
    (unsigned long)b.stepDurationMs,
    (unsigned)b.pulse,
    b.pressHolding ? "true" : "false",
    (unsigned)b.resetPhase,
    (unsigned long)b.batchMs,
    drive(Output::SOLENOID), drive(Output::MIXER_MOTOR), drive(Output::GATE_ACT),
    drive(Output::MIXER_ERM), drive(Output::BOX_ERM), drive(Output::PRESS_ACT),
    b.batteryV, b.solarA
  );
}

// ── Private: _broadcastTelemetry() ───────────────────────────────────────────

void NetworkManager::_broadcastTelemetry() {
  if (_ws.count() == 0) return; // Only construct JSON if we have active clients

  char buf[TELEMETRY_JSON_MAX];
  _buildTelemetryJson(buf, sizeof(buf));

  _ws.textAll(buf);
}

// ── Private: _telemetryTask() ────────────────────────────────────────────────

void NetworkManager::_telemetryTask(void *pvParameters) {
  NetworkManager *instance = static_cast<NetworkManager*>(pvParameters);
  Serial.printf("[RTOS] Telemetry WebSocket task started on CPU core: %d\r\n", xPortGetCoreID());

  for (;;) {
    instance->_broadcastTelemetry();
    vTaskDelay(pdMS_TO_TICKS(1000)); // Update telemetry every 1 second
  }
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
