#pragma once
// ============================================================================
// NextionDashboard.h — Operator Dashboard Drawn on the Nextion at Runtime
//
// Mirrors the web dashboard's Workspace tab on the 800×480 NX8048T070 using
// Nextion drawing instructions (cls / fill / xstr). No custom .HMI needed —
// it only relies on fonts 0 (small) and 1 (large) existing in the loaded
// project, which the factory demo provides.
//
// Screen map (normal):                    Fault overlay:
//   ┌ header: title │ BAT PV CLIENTS UP ┐   red banner, CRITICAL SAFETY HALT,
//   ├ 5-step FSM stepper ───────────────┤   RESET FAULT button
//   ├ stage panel: title, detail,       │
//   │   value, progress bar             │
//   └ 4 KPI tiles │ [ action button ]  ─┘
//
// Usage:
//   NextionDashboard dash(hmi);
//   dash.begin();                          // after hmi.begin()
//   dash.render(machine.snapshot(), ...);  // once per second
//   HmiAction a = dash.handleTouch(x, y);  // on each touch press
// ============================================================================

#include <Arduino.h>
#include "NextionManager.h"
#include "MachineState.h"

struct HmiAction {
  enum Type : uint8_t { NONE, SET_STATE, RESET_FAULT } type;
  uint8_t state;   // target for SET_STATE
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
   * @brief  Draw the dashboard. Full redraw on stage/fault change, otherwise
   *         only fields whose text changed are re-sent.
   */
  void render(const MachineSnapshot &m, uint8_t clients, uint32_t uptimeMs);

  /**
   * @brief  Hit-test a touch press against the action button.
   */
  HmiAction handleTouch(uint16_t x, uint16_t y);

private:
  // Dynamic text fields cached to avoid re-sending unchanged text
  enum Field : uint8_t {
    F_STATUS, F_DETAIL, F_VALUE, F_BUTTON,
    F_KPI0, F_KPI1, F_KPI2, F_KPI3,
    F_COUNT
  };

  struct Rect { uint16_t x, y, w, h; };

  NextionManager &_hmi;
  uint32_t _beganMs;       // when `page 0` was sent
  bool     _pageReady;     // display answered since — safe to draw
  int8_t   _screenKey;     // 0 = fault, 1..5 = stage; -1 = needs full redraw
  char     _cache[F_COUNT][64];
  int16_t  _barFill;       // last progress bar fill width in px (-1 = unknown)

  Rect      _btn;
  bool      _btnEnabled;
  HmiAction _btnAction;
  uint32_t  _lastTouchMs;

  void _invalidate();
  void _fill(const Rect &r, uint16_t color);
  void _text(const Rect &r, uint8_t font, uint16_t fg, uint16_t bg,
             uint8_t align, const char *text);
  void _field(Field f, const Rect &r, uint8_t font, uint16_t fg, uint16_t bg,
              uint8_t align, const char *text);

  void _drawStatic(const MachineSnapshot &m);
  void _drawFaultStatic();
  void _updateStatus(const MachineSnapshot &m, uint8_t clients,
                     uint32_t uptimeMs, uint16_t bg);
  void _updateStage(const MachineSnapshot &m);
  void _updateBar(float fraction);
  void _updateButton(const char *label, bool enabled, HmiAction action);
};
