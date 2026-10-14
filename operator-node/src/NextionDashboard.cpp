// ============================================================================
// NextionDashboard.cpp — Operator Dashboard Drawn on the Nextion at Runtime
//
// See include/NextionDashboard.h for public API documentation.
// ============================================================================

#include "NextionDashboard.h"

namespace {
  // ── Palette: dashboard/src/index.css tokens converted to RGB565 ───────────
  constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return static_cast<uint16_t>(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
  }
  constexpr uint16_t C_BG0     = rgb565(0x0a, 0x0c, 0x10);
  constexpr uint16_t C_BG1     = rgb565(0x12, 0x16, 0x1e);
  constexpr uint16_t C_SURFACE = rgb565(0x1e, 0x25, 0x33);
  constexpr uint16_t C_BORDER  = rgb565(0x2a, 0x34, 0x48);
  constexpr uint16_t C_ACCENT  = rgb565(0x00, 0xd4, 0xaa);
  constexpr uint16_t C_ACC_DIM = rgb565(0x00, 0xa8, 0x82);
  constexpr uint16_t C_WARN    = rgb565(0xf5, 0x9e, 0x0b);
  constexpr uint16_t C_DANGER  = rgb565(0xef, 0x44, 0x44);
  constexpr uint16_t C_TEXT0   = rgb565(0xe8, 0xed, 0xf5);
  constexpr uint16_t C_TEXT1   = rgb565(0x9b, 0xac, 0xc8);
  constexpr uint16_t C_TEXT2   = rgb565(0x5d, 0x72, 0x99);

  // ── Fonts available in the loaded HMI ─────────────────────────────────────
  constexpr uint8_t FONT_SMALL = 0;
  constexpr uint8_t FONT_LARGE = 1;

  // xstr horizontal alignment
  constexpr uint8_t LEFT = 0, CENTER = 1, RIGHT = 2;

  // ── Layout (800 × 480) ────────────────────────────────────────────────────
  constexpr uint16_t SCREEN_W = 800, SCREEN_H = 480;
  constexpr uint16_t MARGIN   = 16;

  constexpr uint16_t HEADER_H = 56;
  constexpr uint16_t STEP_Y = 68, STEP_H = 64, STEP_W = 148, STEP_GAP = 7;
  constexpr uint16_t PANEL_Y = 144, PANEL_H = 256;
  constexpr uint16_t BOTTOM_Y = 412, BOTTOM_H = 56;
  constexpr uint16_t KPI_W = 118, KPI_GAP = 6;

  // Interior of the stage panel
  constexpr uint16_t IN_X = 32, IN_W = SCREEN_W - 2 * IN_X;
  constexpr uint16_t BAR_Y = 324, BAR_H = 24;

  // `page 0` can take seconds on the demo HMI; the display answers nothing
  // until it is done. Give up waiting after this long (no display attached).
  constexpr uint32_t PAGE_LOAD_TIMEOUT_MS = 5000;
  constexpr uint32_t TOUCH_DEBOUNCE = 400;

  struct StageInfo { const char *step; const char *title; };
  constexpr StageInfo STAGES[ProcessConfig::STATE_COUNT] = {
    {"1  SOAK",  "STAGE 1: PAPER SOAKING"},
    {"2  GRIND", "STAGE 2: FIBER GRINDING"},
    {"3  MIX",   "STAGE 3: SLURRY MIXING"},
    {"4  PRESS", "STAGE 4: COMPACTION"},
    {"5  DRY",   "STAGE 5: THERMAL CURING"},
  };

  const char *KPI_LABELS[4] = {"FLOW", "FORCE", "TEMP", "HUMIDITY"};
}

// ── Constructor ──────────────────────────────────────────────────────────────

NextionDashboard::NextionDashboard(NextionManager &hmi)
  : _hmi(hmi),
    _beganMs(0),
    _pageReady(false),
    _screenKey(-1),
    _barFill(-1),
    _btn{0, 0, 0, 0},
    _btnEnabled(false),
    _btnAction{HmiAction::NONE, 0},
    _lastTouchMs(0)
{
  _invalidate();
}

// ── Public: begin() ──────────────────────────────────────────────────────────

void NextionDashboard::begin() {
  _hmi.sendCommand("page 0");
  _hmi.sendCommand("tm0.en=0");   // factory demo: stop the 5 s page slideshow
  _hmi.sendCommand("tsw 255,0");  // ignore the page's own components; we hit-test
  _beganMs   = millis();
  _pageReady = false;
  _invalidate();
}

// ── Public: render() ─────────────────────────────────────────────────────────

void NextionDashboard::render(const MachineSnapshot &m, uint8_t clients,
                              uint32_t uptimeMs) {
  // Commands are processed in order, so any reply after begin() means
  // the page load finished
  if (!_pageReady) {
    _pageReady = (int32_t)(_hmi.lastRxMs() - _beganMs) > 0 ||
                 (millis() - _beganMs) >= PAGE_LOAD_TIMEOUT_MS;
    if (!_pageReady) return;
  }

  const int8_t key = m.safetyOk ? static_cast<int8_t>(m.activeState) : 0;
  if (key != _screenKey) {
    Serial.printf("[HMI ] Full redraw: %s\r\n", key ? STAGES[key - 1].title : "SAFETY HALT");
    _invalidate();
    _screenKey = key;
    if (m.safetyOk) {
      _drawStatic(m);
    } else {
      _drawFaultStatic();
    }
  }

  if (m.safetyOk) {
    _updateStatus(m, clients, uptimeMs, C_BG1);
    _updateStage(m);
  } else {
    _updateStatus(m, clients, uptimeMs, C_DANGER);
  }
}

// ── Public: handleTouch() ────────────────────────────────────────────────────

HmiAction NextionDashboard::handleTouch(uint16_t x, uint16_t y) {
  const uint32_t now = millis();
  if (now - _lastTouchMs < TOUCH_DEBOUNCE) return {HmiAction::NONE, 0};

  const bool inside = x >= _btn.x && x < _btn.x + _btn.w &&
                      y >= _btn.y && y < _btn.y + _btn.h;
  if (!inside || !_btnEnabled) return {HmiAction::NONE, 0};

  _lastTouchMs = now;
  return _btnAction;
}

// ── Private: drawing primitives ──────────────────────────────────────────────

void NextionDashboard::_invalidate() {
  for (auto &c : _cache) c[0] = '\0';
  _barFill    = -1;
  _btnEnabled = false;
  _screenKey  = -1;
}

void NextionDashboard::_fill(const Rect &r, uint16_t color) {
  _hmi.sendf("fill %u,%u,%u,%u,%u", r.x, r.y, r.w, r.h, color);
}

void NextionDashboard::_text(const Rect &r, uint8_t font, uint16_t fg,
                             uint16_t bg, uint8_t align, const char *text) {
  // sta=1: solid background, so redrawing overwrites the old text cleanly
  _hmi.sendf("xstr %u,%u,%u,%u,%u,%u,%u,%u,1,1,\"%s\"",
             r.x, r.y, r.w, r.h, font, fg, bg, align, text);
}

void NextionDashboard::_field(Field f, const Rect &r, uint8_t font, uint16_t fg,
                              uint16_t bg, uint8_t align, const char *text) {
  // Key includes colours so a colour-only change still redraws
  char key[sizeof(_cache[0])];
  snprintf(key, sizeof(key), "%04x%04x%s", fg, bg, text);
  if (strcmp(key, _cache[f]) == 0) return;
  strlcpy(_cache[f], key, sizeof(_cache[f]));
  _text(r, font, fg, bg, align, text);
}

// ── Private: static layout ───────────────────────────────────────────────────

void NextionDashboard::_drawStatic(const MachineSnapshot &m) {
  _hmi.sendf("cls %u", C_BG0);

  // Header
  _fill({0, 0, SCREEN_W, HEADER_H}, C_BG1);
  _fill({0, HEADER_H, SCREEN_W, 1}, C_BORDER);
  _text({MARGIN, 0, 360, HEADER_H}, FONT_SMALL, C_ACCENT, C_BG1, LEFT,
        "BRIQUETTE OPERATOR NODE");

  // FSM stepper: done = dim accent, active = accent, pending = surface
  for (uint8_t i = 0; i < ProcessConfig::STATE_COUNT; ++i) {
    const uint8_t  step = i + 1;
    const bool     active = step == m.activeState;
    const bool     done   = step <  m.activeState;
    const uint16_t bg = active ? C_ACCENT : done ? C_ACC_DIM : C_SURFACE;
    const uint16_t fg = (active || done) ? C_BG0 : C_TEXT2;
    const Rect r{static_cast<uint16_t>(MARGIN + i * (STEP_W + STEP_GAP)),
                 STEP_Y, STEP_W, STEP_H};
    _text(r, FONT_SMALL, fg, bg, CENTER, STAGES[i].step);
  }

  // Stage panel
  _fill({MARGIN, PANEL_Y, SCREEN_W - 2 * MARGIN, PANEL_H}, C_SURFACE);
  _text({IN_X, PANEL_Y + 8, IN_W, 56}, FONT_LARGE, C_TEXT0, C_SURFACE, LEFT,
        STAGES[m.activeState - 1].title);

  // KPI tile frames + labels
  for (uint8_t i = 0; i < 4; ++i) {
    const uint16_t x = MARGIN + i * (KPI_W + KPI_GAP);
    _fill({x, BOTTOM_Y, KPI_W, BOTTOM_H}, C_SURFACE);
    _text({x, BOTTOM_Y, KPI_W, 24}, FONT_SMALL, C_TEXT2, C_SURFACE, CENTER,
          KPI_LABELS[i]);
  }
}

void NextionDashboard::_drawFaultStatic() {
  _hmi.sendf("cls %u", C_BG0);

  _fill({0, 0, SCREEN_W, HEADER_H}, C_DANGER);
  _text({MARGIN, 0, 360, HEADER_H}, FONT_SMALL, C_TEXT0, C_DANGER, LEFT,
        "SAFETY INTERLOCK OPEN");

  _text({0, 140, SCREEN_W, 72}, FONT_LARGE, C_DANGER, C_BG0, CENTER,
        "CRITICAL SAFETY HALT");
  _text({0, 220, SCREEN_W, 36}, FONT_SMALL, C_TEXT0, C_BG0, CENTER,
        "CASING SWITCH TRIGGERED - MOTOR RELAYS OPENED");
  _text({0, 256, SCREEN_W, 36}, FONT_SMALL, C_TEXT1, C_BG0, CENTER,
        "Close the safety cover, then tap RESET FAULT");

  _updateButton("RESET FAULT", true, {HmiAction::RESET_FAULT, 0});
}

// ── Private: dynamic fields ──────────────────────────────────────────────────

void NextionDashboard::_updateStatus(const MachineSnapshot &m, uint8_t clients,
                                     uint32_t uptimeMs, uint16_t bg) {
  const uint32_t s = uptimeMs / 1000UL;
  char buf[48];
  snprintf(buf, sizeof(buf), "BAT %.1fV  PV %.1fA  WIFI %u  %02lu:%02lu:%02lu",
           m.batteryV, m.solarA, (unsigned)clients,
           (unsigned long)(s / 3600UL), (unsigned long)((s / 60UL) % 60UL),
           (unsigned long)(s % 60UL));
  _field(F_STATUS, {380, 0, SCREEN_W - 380 - MARGIN, HEADER_H},
         FONT_SMALL, bg == C_DANGER ? C_TEXT0 : C_TEXT1, bg, RIGHT, buf);
}

void NextionDashboard::_updateStage(const MachineSnapshot &m) {
  char detail[48], value[48], label[24];
  float fraction = -1.0f;   // < 0 hides the bar
  bool  enabled  = true;
  uint16_t valueColor = C_ACCENT;

  switch (m.activeState) {
    case 1:
      snprintf(detail, sizeof(detail), "SOLENOID OPEN   |   GRINDER OFF");
      snprintf(value, sizeof(value), "%.2f L / %.2f L", m.flowL, ProcessConfig::TARGET_FLOW_L);
      fraction = m.flowL / ProcessConfig::TARGET_FLOW_L;
      enabled  = m.flowL >= ProcessConfig::TARGET_FLOW_L;
      snprintf(label, sizeof(label), enabled ? "SOAK DONE" : "FILLING...");
      break;
    case 2:
      snprintf(detail, sizeof(detail), "GRINDER RUNNING   |   MOTOR CURRENT");
      snprintf(value, sizeof(value), "%.1f A", m.grinderA);
      fraction = m.grinderA / ProcessConfig::MAX_GRINDER_A;
      snprintf(label, sizeof(label), "GRIND DONE");
      break;
    case 3:
      snprintf(detail, sizeof(detail), "PLANETARY MIXER ACTIVE   |   60 RPM");
      snprintf(value, sizeof(value), "BLENDING SLURRY");
      snprintf(label, sizeof(label), "MIX DONE");
      break;
    case 4:
      snprintf(detail, sizeof(detail), "ACTUATOR DRIVING DOWN - KEEP HANDS CLEAR");
      snprintf(value, sizeof(value), "%.0f N / %.0f N", m.forceN, ProcessConfig::TARGET_FORCE_N);
      fraction = m.forceN / ProcessConfig::TARGET_FORCE_N;
      enabled  = m.forceN >= ProcessConfig::TARGET_FORCE_N;
      snprintf(label, sizeof(label), enabled ? "CONFIRM MOLD" : "PRESSING...");
      break;
    default:  // 5: drying
      snprintf(detail, sizeof(detail), "ENCLOSURE %.1f C   |   TARGET RH <= %.0f%%",
               m.tempC, ProcessConfig::TARGET_RH_PCT);
      snprintf(value, sizeof(value), "RH %.1f %%", m.rhPct);
      fraction = m.rhPct / 100.0f;
      enabled  = m.rhPct <= ProcessConfig::TARGET_RH_PCT;
      valueColor = enabled ? C_ACCENT : C_WARN;
      snprintf(label, sizeof(label), enabled ? "UNLOAD CHUNK" : "DRYING...");
      break;
  }

  _field(F_DETAIL, {IN_X, PANEL_Y + 68, IN_W, 36}, FONT_SMALL, C_TEXT1, C_SURFACE, LEFT, detail);
  _field(F_VALUE,  {IN_X, PANEL_Y + 108, IN_W, 64}, FONT_LARGE, valueColor, C_SURFACE, LEFT, value);
  _updateBar(fraction);

  // KPI tiles
  char kpi[16];
  const Field fields[4] = {F_KPI0, F_KPI1, F_KPI2, F_KPI3};
  for (uint8_t i = 0; i < 4; ++i) {
    switch (i) {
      case 0: snprintf(kpi, sizeof(kpi), "%.2f L", m.flowL);  break;
      case 1: snprintf(kpi, sizeof(kpi), "%.0f N", m.forceN); break;
      case 2: snprintf(kpi, sizeof(kpi), "%.1f C", m.tempC);  break;
      case 3: snprintf(kpi, sizeof(kpi), "%.1f %%", m.rhPct); break;
    }
    const uint16_t x = MARGIN + i * (KPI_W + KPI_GAP);
    _field(fields[i], {x, BOTTOM_Y + 24, KPI_W, BOTTOM_H - 24},
           FONT_SMALL, C_TEXT0, C_SURFACE, CENTER, kpi);
  }

  const uint8_t next = (m.activeState % ProcessConfig::STATE_COUNT) + 1;
  _updateButton(label, enabled, {HmiAction::SET_STATE, next});
}

void NextionDashboard::_updateBar(float fraction) {
  const Rect track{IN_X, BAR_Y, IN_W, BAR_H};

  if (fraction < 0.0f) {
    if (_barFill != -2) _fill(track, C_SURFACE);   // hidden
    _barFill = -2;
    return;
  }

  const int16_t px = static_cast<int16_t>(constrain(fraction, 0.0f, 1.0f) * IN_W);
  if (px == _barFill) return;
  _barFill = px;

  // Two adjacent fills — no clear-then-draw flicker
  if (px > 0)    _fill({IN_X, BAR_Y, static_cast<uint16_t>(px), BAR_H}, C_ACCENT);
  if (px < IN_W) _fill({static_cast<uint16_t>(IN_X + px), BAR_Y,
                        static_cast<uint16_t>(IN_W - px), BAR_H}, C_BORDER);
}

void NextionDashboard::_updateButton(const char *label, bool enabled, HmiAction action) {
  const bool fault = action.type == HmiAction::RESET_FAULT;
  _btn = fault ? Rect{250, 330, 300, 80}
               : Rect{static_cast<uint16_t>(MARGIN + 4 * (KPI_W + KPI_GAP)), BOTTOM_Y,
                      static_cast<uint16_t>(SCREEN_W - MARGIN - (MARGIN + 4 * (KPI_W + KPI_GAP))),
                      BOTTOM_H};
  _btnEnabled = enabled;
  _btnAction  = action;

  const uint16_t bg = fault ? C_DANGER : enabled ? C_ACCENT : C_SURFACE;
  const uint16_t fg = fault ? C_TEXT0  : enabled ? C_BG0    : C_TEXT2;
  _field(F_BUTTON, _btn, FONT_SMALL, fg, bg, CENTER, label);
}
