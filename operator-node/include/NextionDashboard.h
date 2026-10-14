#pragma once
// ============================================================================
// NextionDashboard.h — Batch-Flow Operator Screens Drawn on the Nextion
//
// Renders mockups/nextion/*.png on the 800×480 NX8048T070 at runtime with
// Nextion drawing instructions (cls / fill / draw / line / cirs / xstr).
// No custom .HMI needed — it relies only on fonts 0 (small) and 1 (large)
// in the loaded project, which the factory demo provides.
//
// Screens follow BatchSnapshot::step:
//   SETUP_PAPER    paper prompt + water vial + 8 level buttons + NEXT
//   SETUP_CHARCOAL setup summary + hopper prompt + BACK / START
//   WATER..RELEASE 10-step bar, step card, outputs panel, STOP
//   UNLOAD         batch summary + DONE
//   STOPPED        red halt screen + RESET & RETRACT
//   RESETTING      homing progress
//
// Full redraw only when the step changes; otherwise only changed fields are
// re-sent (text cache + bar/vial fill cache).
//
// Usage:
//   NextionDashboard dash(hmi);
//   dash.begin();                                   // after hmi.begin()
//   dash.render(process.snapshot(), clients, ms);   // every second
//   HmiCommand c = dash.handleTouch(x, y);          // on each touch press
// ============================================================================

#include <Arduino.h>
#include "NextionManager.h"
#include "BatchProcess.h"

struct HmiCommand {
  bool    valid;
  Cmd     cmd;
  uint8_t arg;
};

class NextionDashboard {
public:
  explicit NextionDashboard(NextionManager &hmi);

  /**
   * @brief  Take over the display (stop demo slideshow, disable component
   *         touch) and schedule a full redraw. Call again after a restart.
   */
  void begin();

  /**
   * @brief  Draw the current screen. Full redraw on step change, otherwise
   *         only fields whose content changed are re-sent.
   */
  void render(const BatchSnapshot &b, uint8_t clients, uint32_t uptimeMs);

  /**
   * @brief  Hit-test a touch press against the buttons on screen.
   */
  HmiCommand handleTouch(uint16_t x, uint16_t y);

private:
  static constexpr uint8_t FIELD_MAX = 24;
  static constexpr uint8_t HIT_MAX   = 10;

  struct Rect { uint16_t x, y, w, h; };
  struct Hit  { Rect r; Cmd cmd; uint8_t arg; };

  NextionManager &_hmi;
  uint32_t _beganMs;       // when `page 0` was sent
  bool     _pageReady;     // display answered since — safe to draw
  int16_t  _screenKey;     // Step as int; -1 = needs full redraw
  char     _cache[FIELD_MAX][64];
  int16_t  _barFill;       // cached progress-bar fill px (-1 = unknown)
  int16_t  _vialFill;      // cached water-vial fill px  (-1 = unknown)

  Hit      _hits[HIT_MAX];
  uint8_t  _hitCount;
  uint32_t _lastTouchMs;
  uint32_t _screenChangedMs;   // last full redraw (touch lockout)

  // ── primitives ─────────────────────────────────────────────────────────
  void _invalidate();
  void _fill(const Rect &r, uint16_t color);
  void _frame(const Rect &r, uint16_t color);
  void _line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
  void _text(const Rect &r, uint8_t font, uint16_t fg, uint16_t bg,
             uint8_t align, const char *text);
  void _field(uint8_t id, const Rect &r, uint8_t font, uint16_t fg, uint16_t bg,
              uint8_t align, const char *text);
  void _button(const Rect &r, uint16_t fg, uint16_t bg, const char *label,
               Cmd cmd, uint8_t arg = 0);
  void _bar(const Rect &track, float fraction, uint16_t color, uint16_t bg);
  void _vial(const Rect &r, uint8_t pct);

  // ── screens ────────────────────────────────────────────────────────────
  void _drawHeader(bool danger);
  void _updateStatus(const BatchSnapshot &b, uint8_t clients, uint32_t uptimeMs, bool danger);

  void _drawSetupPaper(const BatchSnapshot &b);
  void _updateSetupPaper(const BatchSnapshot &b);
  void _drawSetupCharcoal(const BatchSnapshot &b);

  void _drawStepper(uint8_t active);
  void _drawRunning(const BatchSnapshot &b);
  void _updateRunning(const BatchSnapshot &b);
  void _drawOutputsPanel(const BatchSnapshot &b);
  void _updateOutputs(const BatchSnapshot &b);

  void _drawUnload(const BatchSnapshot &b);
  void _drawStopped(const BatchSnapshot &b);
  void _drawResetting();
  void _updateResetting(const BatchSnapshot &b);
};
