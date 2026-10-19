# AUTOBRIQ Project Context

**Project:** AUTOBRIQ: Development of a Fully Automated Paper-Charcoal Briquette Machine
**Date:** 2026-10-19
**Version:** 1.0.0
**Owner:** PUP Computer Engineering Group 6 (Añonuevo, Berongoy, Galdiano, Tiedra)
**Source of truth:** the thesis document (Chapters 1 to 3, October 2026), text copy in [chapters/AUTOBRIQ_fully_automated_thesis.md](../chapters/AUTOBRIQ_fully_automated_thesis.md). Where this suite and the thesis disagree, the thesis wins and this suite is wrong.

This file is the short version for people (and agents) joining the work: what the machine is, what is built, and where the build or the old docs differ from the thesis.

---

## 1. What the machine is (from the thesis)

A solar-powered, **fully automated** paper-charcoal briquette machine. The operator selects the water level, loads shredded paper and crushed charcoal, inserts a mold and presses START. Everything between START and batch completion runs on timers, sensor feedback and safety interlocks, with no mid-cycle prompts. The cycle **ends after compression and press retraction**: there is no thermal drying in the machine, briquettes are air-dried outside it.

| Item | Thesis value |
|------|--------------|
| Output | Solid rectangular-prism briquette, one fixed size, partitioned mold, **at least 20 per batch** |
| Inputs | Paper waste, residual charcoal fines, water (nothing else) |
| Mix | Paper : charcoal about 1 : 1 by weight; wet mix 16.13% to 20% paper, 16.13% to 20% charcoal, 67.7% to 60% water |
| Power | 110 W solar panel, EcoFlow RIVER 2 (256 Wh LiFePO4, MPPT, 12.6 V / 8 A DC outlet, 300 W AC outlet) |
| Mixer power | UME S2415 24 V 15 A supply adjusted to 20 V from the AC outlet, switched by a 30 A automotive relay |
| Controller | ESP32-WROOM-32D (dual core, FreeRTOS), firmware in C++ |
| HMI | Nextion NX8048T070 7.0" touchscreen (UART) + React/Vite PWA over the ESP32 SoftAP |
| Storage | LittleFS (PWA assets), FAT32 Micro-SD (append-only CSV batch log) |
| Study area | Barangay Anglas and Barangay Poblacion, Cavinti, Laguna |

### Hardware list (thesis Tables 6 to 9)

- **Actuators:** two 12 V linear actuators, 1,500 N, 7 mm/s: 15 cm (mixing-container trapdoor) and 40 cm (compression). Two relay channels each (direction).
- **Mixer:** INGCO MXLI2001 20 V paddle mixer, 0 to 650 rpm.
- **Water:** 12 V DC normally closed solenoid valve + YFS-201G1 Hall-effect flow sensor (volume by pulse count, valve closes at the preset volume).
- **Vibration:** two 12 V ERM motors: ERM 1 on the mixing container (discharge), ERM 2 on the compression box (leveling).
- **Force:** 300 kg S-type load cell + HX711 on the compressor block.
- **Position:** three roller-lever limit switches: SW1 trapdoor closed, SW2 compressor home, SW3 mold seated.
- **Relays:** 8-channel 10 A optocoupled board. Channels 1 to 4 actuator directions, **channel 5 water valve, channel 6 the automotive relay for the mixer**, channels 7 and 8 spare.
- **Safety and signalling:** red mushroom E-stop (cuts switched 12 V; controller and screen stay powered), three piezo buzzers, 1N4007 flyback diodes, decoupling capacitors, IP65 enclosure, common ground terminal block (14 AWG power, 22 AWG signal).

### Process (five modules, thesis Figures 4 to 8)

1. **Initialization, loading, batch start:** mount LittleFS and SD, start the SoftAP and PWA, init the Nextion; self-test (interlocks closed, actuators home, power present). Paper screen with water level 30% to 100% (preset 100%), NEXT, charcoal screen, START.
2. **Water and mixing:** valve opens, flow sensor counts to the selected volume, valve closes; mixer runs for a preset time.
3. **Discharge:** mixer stops, coast-down pause, 15 cm actuator retracts (trapdoor opens), **mixer stays off** while ERM 1 shakes the rest down, actuator extends, SW1 confirms closed.
4. **Leveling and compression:** ERM 2 levels the mixture for a preset time; with SW1 and SW3 confirmed the 40 cm actuator extends into the mold while the HX711 is polled; extend relay turns off at the **preset force threshold (set by calibration)**. Unexpected trapdoor opening, mold removal, invalid force reading or timeout goes to fault handling.
5. **Retraction and unload:** actuator retracts until SW2; batch record (runtime, water level, dispensed volume, peak force) is written to the SD as a CSV row and pushed to the PWA; buzzer and screen prompt the operator to pull the mold. SW3 sees the mold leave, the system returns to module 1.

### What the study measures

Man-hours, throughput time, production rate (units/hr), batch yield (target at least 20), fuel performance (burning duration, heat retention vs store-bought charcoal), mass and dimensional variance (30 specimens), 1 m drop test (shatter index), ISO 25010 acceptability (15 end-users, 5-point Likert, weighted mean).

---

## 2. What is built (2026-10-19)

**Main firmware** (`operator-node/src`, `include`): complete batch FSM (`BatchProcess`), Nextion screens drawn at runtime on the 800x480 panel, SoftAP + DNS captive portal + WebSocket `/ws` + `/api/stats`, React PWA in `operator-node/dashboard`. **Outputs are simulated** (`OUTPUTS_LIVE=0`) and the flow and force sensors are simulated.

**Bench rig** (`operator-node/actuator-test`, env `actuator-test`, merged in PR #4): standalone sketch that drives the real relays from the Nextion, a phone page, or serial. Relay map as wired: IN1/IN2 trapdoor (GPIO25/26), IN3/IN4 press (GPIO27/33), IN5 mixer (GPIO23), IN6 water valve (GPIO32). AUTO RUN plays water, mix, trapdoor, mixer burst, close, press, home on timers. HX711 load cell (DT GPIO19, SCK GPIO18) reads and zeroes; the press stops at `PRESS_STOP_KG` (10 kg test value, scale still uncalibrated). Flow sensor code is present but off (`FLOW_ENABLED = false`) because GPIO34 counted noise, flow pin moved to GPIO13.

Wiring, load cell, solenoid, flow sensor and perfboard guides exist as claude.ai artifacts, private to the author's account.

---

## 3. Where the thesis and the build differ

Fix the build to match the thesis unless the thesis is the one that is wrong (section 4).

| # | Thesis says | What exists | Action |
|---|-------------|-------------|--------|
| 1 | Relay channel 5 = water valve, channel 6 = automotive relay for the mixer | Bench rig has mixer on IN5 (relay contact switches the PSU directly) and water on IN6 | Swap the two relay signal jumpers and the two GPIO entries in `actuator_test.cpp`, and add the 30 A automotive relay in the mixer feed, **or** change the thesis table |
| 2 | Main firmware pin map should drive that relay layout | `include/Outputs.h` still has an older map (solenoid 25, mixer 26, gate 27/13, press 18/19, ERMs 32/33) that matches neither the thesis nor the rig | Rewrite `OutputPins` once the relay layout is settled (item 1) |
| 3 | Mixer stays **off** during discharge; ERM 1 shakes the rest down | Firmware has a POUR step that pulses the mixer in bursts | Remove or rework POUR in `BatchProcess` |
| 4 | SW1 to SW3 limit switches gate the cycle and trigger faults | No limit-switch handling in firmware | Add inputs and interlock checks |
| 5 | Three buzzers signal boundaries and faults | Not in firmware | Add buzzer outputs |
| 6 | Batch record to SD as CSV, pushed to the PWA | No SD logging in firmware | Add SD logger (schema in the SDD) |
| 7 | WebSocket telemetry at 10 Hz | Firmware broadcasts once per second | Raise the rate or change the thesis |
| 8 | Core 0 = ISRs and interlocks, Core 1 = networking and telemetry | FSM task is pinned to core 1 (`ProcessConfig::TASK_CORE`) | Decide and align (the Wi-Fi stack itself runs on core 0 by default) |
| 9 | Compression force threshold set by calibration; actuator thrust is 1,500 N | `PRESS_TARGET_N = 2000` in the simulation; bench scale uncalibrated | Calibrate the load cell, then set the real threshold below 1,500 N |
| 10 | SoftAP supports 4 to 5 clients reliably | `AP_MAX_CONN = 8` | Documented as a reliability limit; keep 8 or lower it |
| 11 | PWA self-contained, no CDNs (fonts, icons, CSS) | `index.css` imported Google Fonts | Fixed in this change (system font stack) |
| 12 | Flow sensor drives valve closure at the preset volume | Bench flow sensor off; main firmware simulates flow | Fix the flow signal (moved from GPIO34 to GPIO13; bench noise not yet re-tested) or ship timed water and say so |

## 4. Places where the thesis disagrees with itself

For the authors to settle in the document:

- Table 8 lists a **forced airflow system** ("vents moisture-saturated air during drying") and the block diagram mentions an **exhaust fan**, but the scope says there is no thermal drying.
- The resistors row mentions **DS18B20 1-Wire lines** and the enclosure row mentions a **buck converter**, neither of which appears elsewhere in the hardware list.
- Relay **channels 7 and 8 are "spare"**, yet two ERM vibration motors need switching (the block diagram says the relay module switches the vibration motors).
- The Mix Design table has a "Final Dried Briquette Proportion" column (50% / 50% / 0% water): that is the air-dried state outside the machine, which the machine does not control.
- Equations in the Statistical Treatment are images in the .docx and do not survive text extraction.

---

## 5. Wiring and test guides

Self-contained HTML pages in [guides/](guides/), openable in any browser (they also exist as claude.ai artifacts, but those belong to one account, so these files are the durable copy):

| File | What it covers |
|------|----------------|
| [autobriq-perfboard-layout.html](guides/autobriq-perfboard-layout.html) | Whole-system wiring (EcoFlow, E-stop, relays, loads, USB-A 5 V, bus terminals, 10 A fuse), wire sizes, universal-PCB layout, solder list, parts |
| [autobriq-test-rig-wiring.html](guides/autobriq-test-rig-wiring.html) | Bench rig: ESP32 pin map, actuators, mixer, water, AUTO RUN, bring-up order |
| [autobriq-solenoid-valve.html](guides/autobriq-solenoid-valve.html) | Water valve wiring, flyback diode, dry and wet tests |
| [autobriq-flow-sensor-yfs201.html](guides/autobriq-flow-sensor-yfs201.html) | YF-S201 hookup, test, calibration (code currently off) |
| [autobriq-load-cell-test.html](guides/autobriq-load-cell-test.html) | HX711 and S-type load cell: wiring, zeroing, sign, calibration, press stop |

The relay channels in the rig guides follow the bench rig as wired (CH5 mixer, CH6 water valve), which differs from the thesis (section 3, item 1).

---

## 6. Protected files: do not delete or replace

These are the final operator interfaces. Extend or fix them, but never delete, rename or rewrite them wholesale without the group's agreement:

- **Nextion HMI:** `operator-node/src/NextionDashboard.cpp`, `operator-node/include/NextionDashboard.h`, `operator-node/src/NextionManager.cpp`, `operator-node/include/NextionManager.h`, and the screen mockups in `mockups/nextion/`.
- **PWA dashboard:** everything under `operator-node/dashboard/` (React + Vite, including `src/components/`, `src/hooks/`, `process.js`, `index.css`) and the generated `operator-node/include/WebAssets.h`.

The 2026-10-19 doc sync touched only one CSS import line, one comment and one constant's comment in this area, and deleted nothing.
