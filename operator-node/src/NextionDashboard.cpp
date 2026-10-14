// ============================================================================
// NextionDashboard.cpp — Batch-Flow Operator Screens Drawn on the Nextion
//
// See include/NextionDashboard.h for public API documentation.
// Layout reference: mockups/nextion/screens.html (800×480).
// ============================================================================

#include "NextionDashboard.h"

using namespace ProcessConfig;

namespace {
  // ── Palette: dashboard/src/index.css tokens converted to RGB565 ───────────
  constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return static_cast<uint16_t>(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
  }
  constexpr uint16_t C_BG0     = rgb565(0x0a, 0x0c, 0x10);
  constexpr uint16_t C_BG1     = rgb565(0x12, 0x16, 0x1e);
  constexpr uint16_t C_BG2     = rgb565(0x1a, 0x20, 0x30);
  constexpr uint16_t C_SURFACE = rgb565(0x1e, 0x25, 0x33);
  constexpr uint16_t C_BORDER  = rgb565(0x2a, 0x34, 0x48);
  constexpr uint16_t C_ACCENT  = rgb565(0x00, 0xd4, 0xaa);
  constexpr uint16_t C_ACC_DIM = rgb565(0x00, 0xa8, 0x82);
  constexpr uint16_t C_WARN    = rgb565(0xf5, 0x9e, 0x0b);
  constexpr uint16_t C_DANGER  = rgb565(0xef, 0x44, 0x44);
  constexpr uint16_t C_INFO    = rgb565(0x60, 0xa5, 0xfa);
  constexpr uint16_t C_WATER   = rgb565(0x25, 0x63, 0xeb);
  constexpr uint16_t C_TEXT0   = rgb565(0xe8, 0xed, 0xf5);
  constexpr uint16_t C_TEXT1   = rgb565(0x9b, 0xac, 0xc8);
  constexpr uint16_t C_TEXT2   = rgb565(0x5d, 0x72, 0x99);
  constexpr uint16_t C_PAPER   = rgb565(0xe8, 0xed, 0xf5);
  constexpr uint16_t C_COAL    = rgb565(0x3f, 0x3f, 0x46);

  constexpr uint8_t FONT_SMALL = 0;
  constexpr uint8_t FONT_LARGE = 1;
  constexpr uint8_t LEFT = 0, CENTER = 1, RIGHT = 2;

  // ── Layout (800 × 480) ────────────────────────────────────────────────────
  constexpr uint16_t SCREEN_W = 800;
  constexpr uint16_t M        = 16;           // outer margin
  constexpr uint16_t HEADER_H = 48;

  // Running screens
  constexpr uint16_t STEP_Y = 56, STEP_H = 44, STEP_W = 73, STEP_GAP = 4;
  constexpr uint16_t CARD_Y = 110, CARD_H = 290;
  constexpr uint16_t MAIN_X = M, MAIN_W = 498;
  constexpr uint16_t OUT_X = 522, OUT_W = 262;
  constexpr uint16_t IN_X = MAIN_X + 16, IN_W = MAIN_W - 32;

  // Setup screens
  constexpr uint16_t SET_Y = 58, SET_H = 342;

  // Footer
  constexpr uint16_t FOOT_Y = 410, FOOT_H = 58;

  constexpr uint32_t PAGE_LOAD_TIMEOUT_MS = 5000;
  constexpr uint32_t TOUCH_DEBOUNCE       = 400;
  // After a screen change, ignore taps for this long so a double-tap on one
  // button cannot hit the button that appears in the same spot next
  // (e.g. NEXT then START). STOP is exempt — it must always work.
  constexpr uint32_t SCREEN_LOCK_MS       = 800;

  // ── Field ids (text cache slots) ──────────────────────────────────────────
  enum : uint8_t {
    F_STATUS,
    F_METRIC, F_BAR_L, F_BAR_R,
    F_OUT0,                 // F_OUT0..F_OUT0+5 outputs, +6 load cell
    F_WATER_PCT = F_OUT0 + 7, F_WATER_L,
    F_LVL0,                 // F_LVL0..F_LVL0+7 level buttons
    F_RESET_MSG = F_LVL0 + 8,
    F_COUNT
  };
  static_assert(F_COUNT <= 24, "raise NextionDashboard::FIELD_MAX");

  const char *const STEP_LABELS[10] = {
    "WATER", "MIX", "OPEN", "POUR", "SHAKE", "CLOSE", "LEVEL", "PRESS", "RELEASE", "UNLOAD"
  };

  struct StepText { const char *kicker; const char *title; const char *desc1; const char *desc2; const char *hint; };
  // Indexed by runIndex - 1 (WATER..RELEASE)
  const StepText RUN_TEXT[9] = {
    {"WATER",      "Dispensing water",     "Solenoid valve open. Filling to the",    "selected level of the mixing container.", "Valve closes at the target volume."},
    {"MIX",        "Mixing",               "Mixer motor running through its",        "relay (20V, 5A).",                        "Motor stops when the timer ends."},
    {"OPEN GATE",  "Opening the gate",     "15 cm actuator retracting to open",      "the bottom of the mixing container.",     "Stops at the end of travel."},
    {"POUR",       "Pouring into the box", "Mixer motor spins in short bursts",      "to push the mixture through the gate.",   "Gate stays open. Motor rests between bursts."},
    {"SHAKE",      "Shaking out the rest", "Vibration motor on the mixing",          "container knocks the rest loose.",        "Then the gate closes."},
    {"CLOSE GATE", "Closing the gate",     "15 cm actuator extending to close",      "the mixing container.",                   "Stops at the end of travel."},
    {"LEVEL",      "Leveling the mixture", "Vibration motor on the compression",     "box spreads the mixture evenly.",         "Press starts when the timer ends."},
    {"PRESS",      "Compressing",          "40 cm actuator extending. Force from",   "the S-type load cell (HX711).",           "Holds at target force, then releases."},
    {"RELEASE",    "Releasing the press",  "40 cm actuator retracting so the",       "box can be removed.",                     "Box unlocks at the end of travel."},
  };

  const char *const OUT_NAMES[6] = {
    "Water valve", "Mixer motor", "Gate actuator", "Mixer vibrator", "Box vibrator", "Press actuator"
  };

  constexpr uint8_t LEVELS[8] = {100, 90, 80, 70, 60, 50, 40, 30};

  void fmtMMSS(char *buf, size_t n, uint32_t ms) {
    const uint32_t s = ms / 1000UL;
    snprintf(buf, n, "%02lu:%02lu", (unsigned long)(s / 60UL), (unsigned long)(s % 60UL));
  }
}

// ── Constructor ──────────────────────────────────────────────────────────────

NextionDashboard::NextionDashboard(NextionManager &hmi)
  : _hmi(hmi),
    _beganMs(0),
    _pageReady(false),
    _screenKey(-1),
    _barFill(-1),
    _vialFill(-1),
    _hitCount(0),
    _lastTouchMs(0),
    _screenChangedMs(0)
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

void NextionDashboard::render(const BatchSnapshot &b, uint8_t clients, uint32_t uptimeMs) {
  // Commands are processed in order, so any reply after begin() means
  // the page load finished
  if (!_pageReady) {
    _pageReady = (int32_t)(_hmi.lastRxMs() - _beganMs) > 0 ||
                 (millis() - _beganMs) >= PAGE_LOAD_TIMEOUT_MS;
    if (!_pageReady) return;
  }

  const int16_t key = static_cast<int16_t>(b.step);
  const bool danger = b.step == Step::STOPPED;

  if (key != _screenKey) {
    Serial.printf("[HMI ] Full redraw: %s\r\n", stepName(b.step));
    _invalidate();
    _screenKey = key;
    _screenChangedMs = millis();
    _hmi.sendf("cls %u", C_BG0);
    _drawHeader(danger);

    switch (b.step) {
      case Step::SETUP_PAPER:    _drawSetupPaper(b);    break;
      case Step::SETUP_CHARCOAL: _drawSetupCharcoal(b); break;
      case Step::UNLOAD:         _drawUnload(b);        break;
      case Step::STOPPED:        _drawStopped(b);       break;
      case Step::RESETTING:      _drawResetting();      break;
      default:                   _drawRunning(b);       break;
    }
  }

  _updateStatus(b, clients, uptimeMs, danger);

  switch (b.step) {
    case Step::SETUP_PAPER: _updateSetupPaper(b); break;
    case Step::RESETTING:   _updateResetting(b);  break;
    case Step::SETUP_CHARCOAL:
    case Step::UNLOAD:
    case Step::STOPPED:     break;
    default:                _updateRunning(b);    break;
  }
}

// ── Public: handleTouch() ────────────────────────────────────────────────────

HmiCommand NextionDashboard::handleTouch(uint16_t x, uint16_t y) {
  const uint32_t now = millis();
  if (now - _lastTouchMs < TOUCH_DEBOUNCE) return {false, Cmd::STOP, 0};

  for (uint8_t i = 0; i < _hitCount; ++i) {
    const Rect &r = _hits[i].r;
    if (x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h) {
      if (_hits[i].cmd != Cmd::STOP && now - _screenChangedMs < SCREEN_LOCK_MS) {
        return {false, Cmd::STOP, 0};
      }
      _lastTouchMs = now;
      return {true, _hits[i].cmd, _hits[i].arg};
    }
  }
  return {false, Cmd::STOP, 0};
}

// ── Private: primitives ──────────────────────────────────────────────────────

void NextionDashboard::_invalidate() {
  for (auto &c : _cache) c[0] = '\0';
  _barFill   = -1;
  _vialFill  = -1;
  _hitCount  = 0;
  _screenKey = -1;
}

void NextionDashboard::_fill(const Rect &r, uint16_t color) {
  _hmi.sendf("fill %u,%u,%u,%u,%u", r.x, r.y, r.w, r.h, color);
}

void NextionDashboard::_frame(const Rect &r, uint16_t color) {
  _hmi.sendf("draw %u,%u,%u,%u,%u", r.x, r.y, r.x + r.w - 1, r.y + r.h - 1, color);
}

void NextionDashboard::_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
  _hmi.sendf("line %u,%u,%u,%u,%u", x1, y1, x2, y2, color);
}

void NextionDashboard::_text(const Rect &r, uint8_t font, uint16_t fg,
                             uint16_t bg, uint8_t align, const char *text) {
  // sta=1: solid background, so redrawing overwrites the old text cleanly
  _hmi.sendf("xstr %u,%u,%u,%u,%u,%u,%u,%u,1,1,\"%s\"",
             r.x, r.y, r.w, r.h, font, fg, bg, align, text);
}

void NextionDashboard::_field(uint8_t id, const Rect &r, uint8_t font, uint16_t fg,
                              uint16_t bg, uint8_t align, const char *text) {
  // Key includes colours so a colour-only change still redraws
  char key[sizeof(_cache[0])];
  snprintf(key, sizeof(key), "%04x%04x%s", fg, bg, text);
  if (strcmp(key, _cache[id]) == 0) return;
  strlcpy(_cache[id], key, sizeof(_cache[id]));
  _text(r, font, fg, bg, align, text);
}

void NextionDashboard::_button(const Rect &r, uint16_t fg, uint16_t bg,
                               const char *label, Cmd cmd, uint8_t arg) {
  _text(r, FONT_SMALL, fg, bg, CENTER, label);
  if (_hitCount < HIT_MAX) _hits[_hitCount++] = {r, cmd, arg};
}

void NextionDashboard::_bar(const Rect &t, float fraction, uint16_t color, uint16_t bg) {
  const int16_t px = static_cast<int16_t>(constrain(fraction, 0.0f, 1.0f) * t.w);
  if (px == _barFill) return;
  _barFill = px;
  // Two adjacent fills — no clear-then-draw flicker
  if (px > 0)   _fill({t.x, t.y, static_cast<uint16_t>(px), t.h}, color);
  if (px < t.w) _fill({static_cast<uint16_t>(t.x + px), t.y, static_cast<uint16_t>(t.w - px), t.h}, bg);
}

void NextionDashboard::_vial(const Rect &r, uint8_t pct) {
  // r is the tube interior; the outline is drawn once in the static pass
  const int16_t px = static_cast<int16_t>(r.h * pct / 100);
  if (px == _vialFill) return;
  _vialFill = px;
  if (r.h - px > 0) _fill({r.x, r.y, r.w, static_cast<uint16_t>(r.h - px)}, C_BG2);
  if (px > 0)       _fill({r.x, static_cast<uint16_t>(r.y + r.h - px), r.w, static_cast<uint16_t>(px)}, C_WATER);
}

// ── Private: header ──────────────────────────────────────────────────────────

void NextionDashboard::_drawHeader(bool danger) {
  const uint16_t bg = danger ? C_DANGER : C_BG1;
  _fill({0, 0, SCREEN_W, HEADER_H}, bg);
  if (!danger) _fill({0, HEADER_H, SCREEN_W, 1}, C_BORDER);
  _text({M, 0, 300, HEADER_H}, FONT_SMALL, danger ? C_TEXT0 : C_ACCENT, bg, LEFT,
        "BRIQUETTE OPERATOR NODE");
}

void NextionDashboard::_updateStatus(const BatchSnapshot &b, uint8_t clients,
                                     uint32_t uptimeMs, bool danger) {
  const uint32_t s = uptimeMs / 1000UL;
  char buf[48];
  snprintf(buf, sizeof(buf), "BAT %.1fV   PV %.1fA   WIFI %u   %02lu:%02lu:%02lu",
           b.batteryV, b.solarA, (unsigned)clients,
           (unsigned long)(s / 3600UL), (unsigned long)((s / 60UL) % 60UL),
           (unsigned long)(s % 60UL));
  _field(F_STATUS, {330, 0, SCREEN_W - 330 - M, HEADER_H}, FONT_SMALL,
         danger ? C_TEXT0 : C_TEXT1, danger ? C_DANGER : C_BG1, RIGHT, buf);
}

// ── Private: SETUP 1 — paper + water level ───────────────────────────────────

void NextionDashboard::_drawSetupPaper(const BatchSnapshot &b) {
  (void)b;
  // Left card: paper prompt
  const Rect left{M, SET_Y, 426, SET_H};
  _fill(left, C_SURFACE);
  _text({32, 68, 394, 24}, FONT_SMALL, C_ACCENT, C_SURFACE, LEFT, "SETUP 1 OF 2");
  _text({32, 94, 394, 48}, FONT_LARGE, C_TEXT0, C_SURFACE, LEFT, "Load the paper");
  _text({32, 146, 394, 24}, FONT_SMALL, C_TEXT1, C_SURFACE, LEFT, "Put the shredded paper into the mixing");
  _text({32, 170, 394, 24}, FONT_SMALL, C_TEXT1, C_SURFACE, LEFT, "container, then choose the water level.");

  // Mixing-container icon: trapezoid with two sheets of paper
  const uint16_t cx = 229;
  _fill({cx - 34, 214, 40, 52}, C_PAPER);
  _fill({cx - 6, 206, 40, 52}, C_TEXT1);
  _line(cx - 70, 240, cx + 70, 240, C_TEXT1);
  _line(cx - 70, 240, cx - 56, 330, C_TEXT1);
  _line(cx + 70, 240, cx + 56, 330, C_TEXT1);
  _line(cx - 56, 330, cx + 56, 330, C_TEXT1);
  _text({cx - 90, 338, 180, 22}, FONT_SMALL, C_TEXT2, C_SURFACE, CENTER, "MIXING CONTAINER");
  _text({32, 368, 394, 24}, FONT_SMALL, C_TEXT2, C_SURFACE, LEFT, "Preset 100%   Min 30%");

  // Right card: vial + level buttons
  const Rect right{454, SET_Y, 330, SET_H};
  _fill(right, C_SURFACE);
  _frame({470, 72, 52, 316}, C_TEXT1);                 // vial outline
  for (uint8_t i = 0; i < 8; ++i) {
    const uint8_t v = LEVELS[i];
    const uint16_t y = 74 + 312 - 312 * v / 100;
    _line(522, y, 530, y, C_TEXT2);
    char lbl[6];
    snprintf(lbl, sizeof(lbl), "%u%%", v);
    _text({532, static_cast<uint16_t>(y - 10), 52, 20}, FONT_SMALL, C_TEXT2, C_SURFACE, LEFT, lbl);
  }
  _text({596, 68, 180, 24}, FONT_SMALL, C_ACCENT, C_SURFACE, LEFT, "WATER LEVEL");

  // Footer
  _text({M, FOOT_Y, 450, FOOT_H}, FONT_SMALL, C_TEXT2, C_BG0, LEFT,
        "Tap a level, then press NEXT when the paper is in.");
  _button({484, FOOT_Y, 300, FOOT_H}, C_BG0, C_ACCENT, "PAPER LOADED - NEXT >", Cmd::NEXT);

  // Level buttons register as hits; drawn by _updateSetupPaper()
  for (uint8_t i = 0; i < 8; ++i) {
    const Rect r{static_cast<uint16_t>(596 + (i % 2) * 92),
                 static_cast<uint16_t>(196 + (i / 2) * 48), 86, 40};
    if (_hitCount < HIT_MAX) _hits[_hitCount++] = {r, Cmd::SET_WATER, LEVELS[i]};
  }
}

void NextionDashboard::_updateSetupPaper(const BatchSnapshot &b) {
  char buf[24];
  snprintf(buf, sizeof(buf), "%u%%", (unsigned)b.waterPct);
  _field(F_WATER_PCT, {596, 96, 180, 56}, FONT_LARGE, C_INFO, C_SURFACE, LEFT, buf);
  snprintf(buf, sizeof(buf), "= %.2f L of water", b.targetL);
  _field(F_WATER_L, {596, 154, 180, 24}, FONT_SMALL, C_TEXT2, C_SURFACE, LEFT, buf);

  _vial({472, 74, 48, 312}, b.waterPct);

  for (uint8_t i = 0; i < 8; ++i) {
    const bool sel = LEVELS[i] == b.waterPct;
    const Rect r{static_cast<uint16_t>(596 + (i % 2) * 92),
                 static_cast<uint16_t>(196 + (i / 2) * 48), 86, 40};
    char lbl[6];
    snprintf(lbl, sizeof(lbl), "%u%%", LEVELS[i]);
    _field(F_LVL0 + i, r, FONT_SMALL, sel ? C_BG0 : C_TEXT1, sel ? C_INFO : C_BG2, CENTER, lbl);
  }
}

// ── Private: SETUP 2 — charcoal ──────────────────────────────────────────────

void NextionDashboard::_drawSetupCharcoal(const BatchSnapshot &b) {
  char buf[32];

  // Left: setup-1 summary (the paper card "slid" left)
  const Rect left{M, SET_Y, 220, SET_H};
  _fill(left, C_SURFACE);
  _text({32, 68, 190, 24}, FONT_SMALL, C_TEXT2, C_SURFACE, LEFT, "SETUP 1 - DONE");
  _text({32, 94, 190, 28}, FONT_SMALL, C_TEXT0, C_SURFACE, LEFT, "Paper loaded");
  _frame({104, 132, 44, 196}, C_TEXT1);
  _vialFill = -1;
  _vial({106, 134, 40, 192}, b.waterPct);
  snprintf(buf, sizeof(buf), "Water %u%%  (%.2f L)", (unsigned)b.waterPct, b.targetL);
  _text({32, 336, 190, 24}, FONT_SMALL, C_ACCENT, C_SURFACE, LEFT, buf);
  _text({32, 364, 190, 24}, FONT_SMALL, C_TEXT2, C_SURFACE, LEFT, "BACK to change level");

  // Right: hopper prompt
  const Rect right{248, SET_Y, 536, SET_H};
  _fill(right, C_SURFACE);
  _text({264, 68, 500, 24}, FONT_SMALL, C_ACCENT, C_SURFACE, LEFT, "SETUP 2 OF 2");
  _text({264, 94, 500, 48}, FONT_LARGE, C_TEXT0, C_SURFACE, LEFT, "Load the charcoal");
  _text({264, 146, 500, 24}, FONT_SMALL, C_TEXT1, C_SURFACE, LEFT, "Pour the crushed charcoal into the hopper.");
  _text({264, 170, 500, 24}, FONT_SMALL, C_TEXT1, C_SURFACE, LEFT, "Press START when everything is loaded.");

  // Hopper icon: funnel outline with charcoal bits
  const uint16_t hx = 516;
  _line(hx - 80, 208, hx + 80, 208, C_TEXT1);
  _line(hx - 80, 208, hx - 22, 290, C_TEXT1);
  _line(hx + 80, 208, hx + 22, 290, C_TEXT1);
  _line(hx - 22, 290, hx - 22, 326, C_TEXT1);
  _line(hx + 22, 290, hx + 22, 326, C_TEXT1);
  _line(hx - 22, 326, hx + 22, 326, C_TEXT1);
  const int16_t bits[][2] = {{-50,222},{-28,226},{-4,220},{22,228},{46,222},{-30,246},{-6,244},{18,250},{-12,266},{8,270}};
  for (auto &p : bits) _hmi.sendf("cirs %d,%d,5,%u", hx + p[0], p[1], C_COAL);
  _text({hx - 90, 334, 180, 22}, FONT_SMALL, C_TEXT2, C_SURFACE, CENTER, "CHARCOAL HOPPER");
  snprintf(buf, sizeof(buf), "[OK] Paper   [OK] Water %u%%", (unsigned)b.waterPct);
  _text({264, 364, 500, 24}, FONT_SMALL, C_ACCENT, C_SURFACE, LEFT, buf);

  // Footer
  _button({M, FOOT_Y, 150, FOOT_H}, C_TEXT1, C_SURFACE, "< BACK", Cmd::BACK);
  _text({180, FOOT_Y, 290, FOOT_H}, FONT_SMALL, C_TEXT2, C_BG0, CENTER, "Close the lid before starting.");
  _button({484, FOOT_Y, 300, FOOT_H}, C_BG0, C_ACCENT, "START PROCESS", Cmd::START);
}

// ── Private: running steps ───────────────────────────────────────────────────

void NextionDashboard::_drawStepper(uint8_t active) {
  for (uint8_t i = 0; i < 10; ++i) {
    const uint8_t n = i + 1;
    const uint16_t bg = n < active ? C_ACC_DIM : n == active ? C_ACCENT : C_SURFACE;
    const uint16_t fg = n <= active ? C_BG0 : C_TEXT2;
    const uint16_t x = M + i * (STEP_W + STEP_GAP);
    char num[4];
    snprintf(num, sizeof(num), "%u", n);
    _text({x, STEP_Y, STEP_W, 20}, FONT_SMALL, fg, bg, CENTER, num);
    _text({x, STEP_Y + 20, STEP_W, STEP_H - 20}, FONT_SMALL, fg, bg, CENTER, STEP_LABELS[i]);
  }
}

void NextionDashboard::_drawRunning(const BatchSnapshot &b) {
  _drawStepper(b.runIndex);

  const StepText &t = RUN_TEXT[b.runIndex - 1];
  _fill({MAIN_X, CARD_Y, MAIN_W, CARD_H}, C_SURFACE);

  char kicker[32];
  snprintf(kicker, sizeof(kicker), "STEP %u - %s", (unsigned)b.runIndex, t.kicker);
  _text({IN_X, 118, IN_W, 24}, FONT_SMALL, C_ACCENT, C_SURFACE, LEFT, kicker);
  _text({IN_X, 142, IN_W, 48}, FONT_LARGE, C_TEXT0, C_SURFACE, LEFT, t.title);
  _text({IN_X, 192, IN_W, 22}, FONT_SMALL, C_TEXT1, C_SURFACE, LEFT, t.desc1);
  _text({IN_X, 214, IN_W, 22}, FONT_SMALL, C_TEXT1, C_SURFACE, LEFT, t.desc2);
  if (b.step == Step::PRESS) {
    _text({IN_X, 368, IN_W, 24}, FONT_SMALL, C_WARN, C_SURFACE, LEFT, "! KEEP HANDS CLEAR OF THE PRESS");
  }

  _drawOutputsPanel(b);

  // Footer
  char hint[64];
  snprintf(hint, sizeof(hint), "Runs automatically. %s", t.hint);
  _text({M, FOOT_Y, 550, FOOT_H}, FONT_SMALL, C_TEXT2, C_BG0, LEFT, hint);
  _frame({584, FOOT_Y, 200, FOOT_H}, C_DANGER);
  _frame({585, FOOT_Y + 1, 198, FOOT_H - 2}, C_DANGER);
  _button({587, FOOT_Y + 2, 194, FOOT_H - 4}, C_DANGER, C_BG0, "STOP", Cmd::STOP);
}

void NextionDashboard::_updateRunning(const BatchSnapshot &b) {
  char metric[32], l[32], r[24], a[8], c[8];
  float frac = b.stepDurationMs ? static_cast<float>(b.stepElapsedMs) / b.stepDurationMs : 0.0f;
  uint16_t barColor = C_ACCENT;

  switch (b.step) {
    case Step::WATER:
      snprintf(metric, sizeof(metric), "%.2f L / %.2f L", b.flowL, b.targetL);
      frac = b.targetL > 0 ? b.flowL / b.targetL : 0.0f;
      snprintf(l, sizeof(l), "Target: %u%% level", (unsigned)b.waterPct);
      snprintf(r, sizeof(r), "%d%%", (int)(constrain(frac, 0.0f, 1.0f) * 100));
      break;
    case Step::MIX:
    case Step::SHAKE:
    case Step::LEVEL:
      fmtMMSS(a, sizeof(a), b.stepDurationMs - min(b.stepElapsedMs, b.stepDurationMs));
      if (b.step == Step::MIX) snprintf(metric, sizeof(metric), "%s left   %.1f A", a, b.mixerA);
      else                     snprintf(metric, sizeof(metric), "%s left", a);
      fmtMMSS(a, sizeof(a), b.stepElapsedMs);
      fmtMMSS(c, sizeof(c), b.stepDurationMs);
      snprintf(l, sizeof(l), "%s elapsed", a);
      snprintf(r, sizeof(r), "%s total", c);
      break;
    case Step::OPEN_GATE:
    case Step::CLOSE_GATE:
    case Step::RELEASE:
      snprintf(metric, sizeof(metric), "%.1f s", b.stepElapsedMs / 1000.0f);
      snprintf(l, sizeof(l), "Full stroke");
      snprintf(r, sizeof(r), "~%lu s", (unsigned long)((b.stepDurationMs + 999) / 1000));
      break;
    case Step::POUR:
      snprintf(metric, sizeof(metric), "%u / %u bursts", (unsigned)b.pulse, (unsigned)POUR_PULSES);
      snprintf(l, sizeof(l), "%lu s on / %lu s off",
               (unsigned long)(POUR_ON_MS * DIV / 1000), (unsigned long)(POUR_OFF_MS * DIV / 1000));
      snprintf(r, sizeof(r), b.pulseOn ? "MOTOR ON" : "RESTING");
      break;
    case Step::PRESS:
      snprintf(metric, sizeof(metric), "%.0f N / %.0f N", b.forceN, PRESS_TARGET_N);
      frac = b.forceN / PRESS_TARGET_N;
      barColor = C_WARN;
      snprintf(l, sizeof(l), "Load cell (S-type + HX711)");
      if (b.pressHolding) snprintf(r, sizeof(r), "HOLDING");
      else                snprintf(r, sizeof(r), "%d%%", (int)(constrain(frac, 0.0f, 1.0f) * 100));
      break;
    default:
      return;
  }

  _field(F_METRIC, {IN_X, 244, IN_W, 60}, FONT_LARGE, C_ACCENT, C_SURFACE, LEFT, metric);
  _bar({IN_X, 312, IN_W, 22}, frac, barColor, C_BORDER);
  _field(F_BAR_L, {IN_X, 338, IN_W / 2 + 60, 24}, FONT_SMALL, C_TEXT2, C_SURFACE, LEFT, l);
  _field(F_BAR_R, {IN_X + IN_W / 2 + 60, 338, IN_W / 2 - 60, 24}, FONT_SMALL, C_TEXT2, C_SURFACE, RIGHT, r);

  _updateOutputs(b);
}

// ── Private: outputs panel ───────────────────────────────────────────────────

void NextionDashboard::_drawOutputsPanel(const BatchSnapshot &b) {
  _fill({OUT_X, CARD_Y, OUT_W, CARD_H}, C_SURFACE);
  _text({OUT_X + 10, 116, OUT_W - 20, 24}, FONT_SMALL, C_ACCENT, C_SURFACE, LEFT, "OUTPUTS - ONE AT A TIME");

  // Which output this step owns (highlighted for the whole step)
  int8_t owner = -1;
  switch (b.step) {
    case Step::WATER:      owner = 0; break;
    case Step::MIX:
    case Step::POUR:       owner = 1; break;
    case Step::OPEN_GATE:
    case Step::CLOSE_GATE: owner = 2; break;
    case Step::SHAKE:      owner = 3; break;
    case Step::LEVEL:      owner = 4; break;
    case Step::PRESS:
    case Step::RELEASE:    owner = 5; break;
    default: break;
  }

  for (uint8_t i = 0; i < 7; ++i) {
    const uint16_t y = i < 6 ? 144 + i * 34 : 360;
    _fill({OUT_X + 8, y, OUT_W - 16, 30}, C_BG2);
    if (i == owner) _frame({OUT_X + 7, static_cast<uint16_t>(y - 1), OUT_W - 14, 32}, C_ACCENT);
    _text({OUT_X + 16, y, 150, 30}, FONT_SMALL, i == owner ? C_TEXT0 : C_TEXT1, C_BG2, LEFT,
          i < 6 ? OUT_NAMES[i] : "Load cell");
  }
  _line(OUT_X + 8, 352, OUT_X + OUT_W - 8, 352, C_BORDER);
}

void NextionDashboard::_updateOutputs(const BatchSnapshot &b) {
  // Gate position follows the step sequence (open between OPEN and CLOSE)
  const bool gateOpen = b.step == Step::POUR || b.step == Step::SHAKE;

  for (uint8_t i = 0; i < 7; ++i) {
    const uint16_t y = i < 6 ? 144 + i * 34 : 360;
    const char *label = "OFF";
    uint16_t bg = C_BORDER, fg = C_TEXT2;

    if (i < 6) {
      const Drive d = b.outputs[i];
      switch (i) {
        case 0: label = d == Drive::ON ? "OPEN" : "CLOSED"; break;
        case 1:
          if (b.step == Step::POUR) label = d == Drive::ON ? "PULSE" : "REST";
          else                      label = d == Drive::ON ? "ON" : "OFF";
          break;
        case 2:
          label = d == Drive::EXTEND ? "EXTENDING" : d == Drive::RETRACT ? "RETRACTING"
                : gateOpen ? "OPEN" : "CLOSED";
          break;
        case 5:
          label = d == Drive::EXTEND ? "EXTENDING" : d == Drive::RETRACT ? "RETRACTING"
                : b.pressHolding ? "HOLDING" : "RETRACTED";
          break;
        default: label = d == Drive::ON ? "ON" : "OFF"; break;
      }
      if (d == Drive::EXTEND || d == Drive::RETRACT)            { bg = C_WARN;   fg = C_BG0; }
      else if (d == Drive::ON && b.step == Step::POUR && i == 1) { bg = C_INFO;   fg = C_BG0; }
      else if (d == Drive::ON)                                   { bg = C_ACCENT; fg = C_BG0; }
      else if (b.pressHolding && i == 5)                         { bg = C_WARN;   fg = C_BG0; }
    } else {
      static char force[12];
      const bool active = b.step == Step::PRESS || b.step == Step::RELEASE;
      snprintf(force, sizeof(force), "%.0f N", b.forceN);
      label = active ? force : "IDLE";
      if (active) { bg = C_ACCENT; fg = C_BG0; }
    }

    _field(F_OUT0 + i, {OUT_X + OUT_W - 110, static_cast<uint16_t>(y + 4), 96, 22},
           FONT_SMALL, fg, bg, CENTER, label);
  }
}

// ── Private: UNLOAD ──────────────────────────────────────────────────────────

void NextionDashboard::_drawUnload(const BatchSnapshot &b) {
  _drawStepper(10);
  _fill({MAIN_X, CARD_Y, MAIN_W, CARD_H}, C_SURFACE);
  _text({IN_X, 118, IN_W, 24}, FONT_SMALL, C_ACCENT, C_SURFACE, LEFT, "STEP 10 - UNLOAD");
  _text({IN_X, 142, IN_W, 48}, FONT_LARGE, C_TEXT0, C_SURFACE, LEFT, "Briquette ready");
  _text({IN_X, 192, IN_W, 22}, FONT_SMALL, C_TEXT1, C_SURFACE, LEFT, "All motors are off. Remove the press box and");
  _text({IN_X, 214, IN_W, 22}, FONT_SMALL, C_TEXT1, C_SURFACE, LEFT, "put the briquette in your drying container.");

  char v[4][16];
  char mix[8], total[8];
  fmtMMSS(mix, sizeof(mix), MIX_MS);
  fmtMMSS(total, sizeof(total), b.batchMs);
  snprintf(v[0], sizeof(v[0]), "%.2f L", b.targetL);
  snprintf(v[1], sizeof(v[1]), "%s", mix);
  snprintf(v[2], sizeof(v[2]), "%.0f N", b.peakForceN);
  snprintf(v[3], sizeof(v[3]), "%s", total);
  const char *k[4] = {"WATER", "MIX", "PEAK FORCE", "TOTAL"};
  constexpr uint16_t TW = 108, TG = 6;
  for (uint8_t i = 0; i < 4; ++i) {
    const uint16_t x = IN_X + i * (TW + TG);
    _fill({x, 316, TW, 72}, C_BG2);
    _text({x, 320, TW, 24}, FONT_SMALL, C_TEXT2, C_BG2, CENTER, k[i]);
    _text({x, 346, TW, 36}, FONT_SMALL, C_TEXT0, C_BG2, CENTER, v[i]);
  }

  _drawOutputsPanel(b);
  _updateOutputs(b);

  _text({M, FOOT_Y, 450, FOOT_H}, FONT_SMALL, C_TEXT2, C_BG0, LEFT,
        "Put the empty box back before the next batch.");
  _button({484, FOOT_Y, 300, FOOT_H}, C_BG0, C_ACCENT, "DONE - NEW BATCH", Cmd::DONE);
}

// ── Private: STOPPED / RESETTING ─────────────────────────────────────────────

void NextionDashboard::_drawStopped(const BatchSnapshot &b) {
  _text({0, 150, SCREEN_W, 64}, FONT_LARGE, C_DANGER, C_BG0, CENTER, "PROCESS STOPPED");
  _text({0, 224, SCREEN_W, 28}, FONT_SMALL, C_TEXT0, C_BG0, CENTER,
        "STOP pressed or safety cover opened. All relays switched off.");
  char buf[48];
  snprintf(buf, sizeof(buf), "Stopped during: %s", stepName(b.stoppedDuring));
  _text({0, 256, SCREEN_W, 28}, FONT_SMALL, C_TEXT1, C_BG0, CENTER, buf);
  _button({250, 330, 300, 72}, C_TEXT0, C_DANGER, "RESET & RETRACT", Cmd::RESET);
}

void NextionDashboard::_drawResetting() {
  _text({0, 140, SCREEN_W, 64}, FONT_LARGE, C_WARN, C_BG0, CENTER, "RESETTING");
  _text({0, 284, SCREEN_W, 28}, FONT_SMALL, C_TEXT2, C_BG0, CENTER,
        "Returning the press and gate to their start positions. Keep clear.");
}

void NextionDashboard::_updateResetting(const BatchSnapshot &b) {
  _field(F_RESET_MSG, {0, 212, SCREEN_W, 32}, FONT_SMALL, C_TEXT0, C_BG0, CENTER,
         b.resetPhase == 0 ? "Step 1 of 2: retracting the press" : "Step 2 of 2: closing the gate");
  const float frac = b.stepDurationMs ? static_cast<float>(b.stepElapsedMs) / b.stepDurationMs : 0.0f;
  _bar({150, 252, 500, 22}, frac, C_WARN, C_BORDER);
}
