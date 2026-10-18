# Nextion HMI Touchscreen Design Guide (`Nextion_HMI_Design.md`)

This document describes the on-machine operator console: a **Nextion NX8048T070 7.0" Intelligent screen, 800 × 480**. The firmware draws every screen at runtime with Nextion drawing instructions, so no custom `.HMI` project has to be designed in the Nextion Editor. Pixel mockups are in `mockups/nextion/`, and the drawing code is `operator-node/src/NextionDashboard.cpp`.

Per the thesis, the touchscreen provides batch-start controls, process status, water-volume and compression-force readings, and fault notifications. Deeper data (history, CSV download) is on the PWA ([PRD](PRD_Template.md)).

---

## 1. Global Touchscreen Configuration

*   **Resolution:** 800 × 480 pixels (landscape).
*   **Wiring:** Nextion TX to ESP32 GPIO16 (RX2), Nextion RX to ESP32 GPIO17 (TX2), shared ground. The panel needs its own 5 V supply: it draws up to about 500 mA, more than a USB port can give together with the ESP32's Wi-Fi.
*   **Baud rate:** The Nextion boots at **9600**. The firmware asks it to switch to **115200** at start-up and falls back to resync if the display goes quiet (the setting is runtime-only and reverts on power-off).
*   **Fonts:** Only font 0 (small) and font 1 (large) from the loaded project are used; the factory demo project provides them.
*   **Takeover:** On start the firmware stops the factory demo slideshow (`tm0.en=0`), disables the page's own touch components (`tsw 255,0`) and enables raw touch reports (`sendxy=1`). Touch is hit-tested in firmware against the buttons it drew.
*   **Theme palette** (same tokens as the PWA, see [DSD](DSD_Template.md)):
    *   Background: `#0A0C10` (panels `#12161E`, `#1E2533`)
    *   Accent / active: Neon teal `#00D4AA`
    *   Warning: `#F59E0B`
    *   Danger / halt: Safety red `#EF4444`
    *   Text: `#E8EDF5` (secondary `#9BACC8`, muted `#5D7299`)

**Layout grid:** 16 px outer margin, 48 px header, running screens have a 10-step bar (y 56), a step card (498 px wide) with an outputs panel (262 px wide) on the right, and a 58 px footer at y 410 for the main action.

---

## 2. Screens

The screen shown follows the state of the batch FSM. A full redraw happens only when the step changes; otherwise only the fields that changed are re-sent.

### Setup 1: paper and water level (`SETUP_PAPER`)
*   **Purpose:** Operator loads shredded paper and sets the water level.
*   **Content:** Paper prompt, a vial-shaped water indicator, eight level buttons (100, 90, 80, 70, 60, 50, 40, 30 percent; preset 100) with the volume in liters, and `PAPER LOADED - NEXT >`.
*   **Action:** NEXT keeps the chosen level and slides to Setup 2.

### Setup 2: charcoal and start (`SETUP_CHARCOAL`)
*   **Purpose:** Operator loads crushed charcoal into the hopper and starts the batch.
*   **Content:** Summary of the setup, hopper prompt, `< BACK`, the note "Close the lid before starting.", and `START PROCESS`.
*   **Action:** START begins the automatic cycle. After this there are no confirmations until the mold prompt.

### Running (`WATER` to `RELEASE`)
*   **Purpose:** Show the automatic cycle. The operator only watches.
*   **Content:** Header with status and connected clients; a 10-step bar (WATER, MIX, OPEN, POUR, SHAKE, CLOSE, LEVEL, PRESS, RELEASE, UNLOAD); a step card with title, description, a progress bar and a live metric (time left, liters, or force); an outputs panel listing the water valve, mixer motor, gate actuator, mixer vibrator, box vibrator, press actuator and the load cell reading; a red `STOP` button in the footer.
*   **Action:** STOP switches every output off and opens the halt screen.

### Unload (`UNLOAD`)
*   **Purpose:** Batch summary and mold removal.
*   **Content:** Batch summary (water time, mix time, peak force, total time) and `DONE - NEW BATCH`.
*   **Thesis note:** The thesis has the buzzer sound and the screen prompt the operator to remove the pull-out mold, and the system return to setup when the mold-seated switch (SW3) detects removal. The current firmware returns on the DONE button.

### Stopped (`STOPPED`)
*   **Purpose:** Halt screen after STOP or a safety fault.
*   **Content:** Red header, a message that all relays are off, and `RESET & RETRACT`.
*   **Action:** RESET retracts the press and closes the trapdoor, then returns to Setup 1.

### Resetting (`RESETTING`)
*   **Purpose:** Homing progress after RESET. Taps are ignored until it finishes.

---

## 3. Touch and Link Behavior

*   **Touch frames:** The display reports presses as coordinate frames (`0x67 x y state`, terminated by `0xFF 0xFF 0xFF`). The firmware matches the point against the button rectangles it drew for the current screen.
*   **Debounce:** 400 ms between accepted presses.
*   **Screen lock:** After any screen change, taps are ignored for 800 ms so a double tap cannot hit the next screen's button in the same spot (for example NEXT then START). STOP is exempt and always works.
*   **Flow control:** `bkcmd=3` makes the display acknowledge every command; at most four commands are outstanding so its input buffer never overruns during a full redraw.
*   **Keepalive:** The firmware pings (`sendme`) every 2 seconds and resyncs the link and redraws after 5 seconds of silence, which covers a display reboot or an unplugged cable.
*   **Packets out:** Commands such as `cls`, `fill`, `draw`, `xstr` end with `0xFF 0xFF 0xFF`. There are no `state=` or `grind=` messages: the display sends only touch coordinates.

---

## 4. Thesis Requirements Not Yet On Screen

| Thesis requirement | Status |
|--------------------|--------|
| Dedicated fault screen with buzzer for an opened trapdoor (SW1), removed mold (SW3), invalid force reading or compression timeout | Not yet: STOP and RESET exist; limit-switch faults are not wired in |
| Mold-removal prompt with buzzer, cleared by SW3 | Not yet: DONE button instead |
| Fault notifications on the touchscreen for interlock and sensor failures | Partly: halt screen only |

The authoritative gap list is in [PROJECT_CONTEXT.md](PROJECT_CONTEXT.md) section 3.
