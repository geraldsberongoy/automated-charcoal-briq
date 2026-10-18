# QA & Test Plan (QAD)

**Project:** AUTOBRIQ Fully Automated Paper-Charcoal Briquette Machine
**Date:** 2026-10-19
**Version:** 1.1.0
**Owner:** PUP Computer Engineering Group 6 (Añonuevo, Berongoy, Galdiano, Tiedra)
**PRD:** [PRD_Template.md](PRD_Template.md)

---

> **Context:** "Launch" for this plan equals the **functional prototype demonstration and defense gate (October 2026)**, followed by the guided evaluation in Cavinti. A passing result means the machine runs a full automatic batch cycle safely, stops at its force threshold, halts on interlock faults, and shows readings on both interfaces.

---

## 1. Testing Strategy & Scope

**In Scope:**
*   **Automatic Cycle:** From START to the mold-removal prompt with no operator input: water, mixing, discharge, leveling, compression, retraction.
*   **Water Volume Cutoff:** The valve closes when the YF-S201 pulse count reaches the volume for the selected level.
*   **Compression Stop:** The extend relay turns off at the preset force threshold from the HX711 and 300 kg load cell; the press then retracts until SW2.
*   **Interlock Faults:** SW1 (trapdoor), SW3 (mold), invalid force reading, compression timeout, and the E-stop.
*   **Offline PWA:** Registers `/sw.js` and loads fully with no internet; no external CDN requests.
*   **Batch Record:** One CSV row per batch on the SD card.

**Out of Scope (this gate):**
*   **Drying:** The machine has no drying stage; air-drying is outside the system.
*   **Multi-machine coordination.**
*   **Enterprise security audits:** The SoftAP has no WAN connection and a range of about 15 m.

**Testing Levels:**

| Level | Tooling | Owner |
|-------|---------|-------|
| **Unit Testing** | PlatformIO unit tests (planned; none written yet) | Berongoy |
| **Integration Testing** | Bench rig with the real relays and sensors (`actuator-test` sketch, serial log) | Añonuevo |
| **System E2E Testing** | Manual full cycles on the assembled machine | Group 6 |

---

## 2. Test Environments & Data

*   **Physical Staging:** Assembled prototype powered by the EcoFlow RIVER 2 (12 V DC outlet for actuators and control, AC outlet for the mixer supply set to 20 V), with the 110 W panel when sun is available.
*   **Dashboard Staging:** Phones (Android 12+, iOS 16+) connected to SSID `Operator-Node-01` (`http://192.168.4.1`); browser dev tools for service-worker checks.
*   **Test Data Setup:**
    *   The main firmware runs with simulated flow and force sensors and simulated outputs (`OUTPUTS_LIVE=0`) until the real drivers are enabled.
    *   Flow: inject pulses or run water through the sensor into a measuring jug.
    *   Force: apply known weights to the load cell (it must be calibrated first).

---

## 3. Core Test Scenarios

### Happy Paths

| ID | Scenario | Steps | Expected Result | US-ID |
|----|----------|-------|-----------------|-------|
| **H-01** | Offline Dashboard Launch | 1. Power the ESP32.<br>2. Disable mobile data on the phone.<br>3. Connect to `Operator-Node-01`.<br>4. Open `http://192.168.4.1`. | The dashboard loads, the service worker registers, and no request goes to an external host. | US-02 |
| **H-02** | Water Volume Cutoff | 1. Select a water level and START.<br>2. Valve opens.<br>3. Inject or flow pulses. | The valve closes when the pulse count reaches the volume for that level (jug check within the calibration margin). | US-01 |
| **H-03** | Compression Stop | 1. Reach the compression step.<br>2. The 40 cm actuator extends.<br>3. Load the cell past the threshold. | The extend relay turns off at the threshold; the actuator retracts until SW2 and the retract relay turns off. | US-03 |
| **H-04** | Full Automatic Cycle | 1. Load materials, seat the mold, START.<br>2. Do nothing until the mold prompt. | All five phases run with no confirmation requests; buzzer and "remove the mold" prompt at the end. | US-01, US-02 |
| **H-05** | Mold Removal Returns to Setup | 1. At the prompt, remove the mold. | SW3 opens, the system returns to the first setup screen; a new batch needs a seated mold and START. | US-02 |
| **H-06** | Batch Record | 1. Complete a batch.<br>2. Remove the SD card and open the CSV. | One new row with `batch_id`, `water_level_pct`, `water_vol_l`, `runtime_s`, `peak_force_n`, `status`; the PWA history shows it. | US-05 |

### Sad Paths

| ID | Scenario | Input / Trigger | Expected Behavior |
|----|----------|-----------------|-------------------|
| **S-01** | Trapdoor opens during compression | Open SW1 while the press extends. | Production loads switch off, fault screen and buzzer on the Nextion, fault alert on the PWA. Recovery only after the cause is cleared and RESET is pressed. |
| **S-02** | Mold removed mid-cycle | Open SW3 during leveling or compression. | Same fault handling as S-01. |
| **S-03** | Sensor disconnected | Unplug the HX711 or the flow sensor. | A visible warning beside the reading; the cycle does not continue past the step that needs it (invalid force reading is a fault). |
| **S-04** | Compression timeout | Hold the load cell below the threshold. | The extend relay turns off at the timeout and the system goes to the fault state. |
| **S-05** | Emergency stop | Press the E-stop mid-cycle. | Switched 12 V drops, actuators and valve stop; the controller and touchscreen stay on and show the stopped state. |
| **S-06** | Too many clients | Connect more than 5 phones and poll continuously. | Heap stays stable, the server does not restart, and control from the Nextion keeps working. |
| **S-07** | Self-test failure | Power on with an actuator away from home or an interlock open. | Fault state with on-screen alert and buzzer; START is refused. |

---

## 4. Automation vs. Manual Testing

### Automated Gate (what exists today)
```yaml
# Build checks
- run: pio run -e esp32doit-devkit-v1     # main firmware compiles
- run: pio run -e actuator-test           # relay bench sketch compiles
- run: npm run build   # in operator-node/dashboard; PWA builds
- GATE: all three build with 0 errors
```
Unit tests (PlatformIO and dashboard) are planned; no test suites exist yet.

### Manual Verification
*   **Interlock Check:** Trigger SW1, SW2, SW3 and the E-stop before every operating session.
*   **Sensor Calibration:** Zero and scale the load cell with a known weight; check the flow sensor against a measuring jug; record both.
*   **Mobile Browser Spot Checks:** Chrome (Android) and Safari (iOS) under direct sunlight for visibility and button response.

### Study Measurements (research instruments, not QA gates)

| Measurement | Instrument | Sample |
|-------------|------------|--------|
| Man-hours and throughput time | Digital stopwatch, observation logs | Manual baseline and machine runs with 15 end-users |
| Production rate and batch yield | Batch counter, operating time | Every batch; target at least 20 briquettes |
| Mass and dimensional variance | Digital scale, Vernier caliper | 30 machine-made briquettes |
| Structural integrity | 1 m drop test, shatter-resistance index | Same 30 briquettes |
| Fuel performance | Stopwatch, infrared thermometer | Burning duration and heat retention vs store-bought charcoal |
| Acceptability | ISO 25010 questionnaire, 5-point Likert | 15 end-users after guided runs |

---

## 5. Bug Triage Protocol

| Severity | Definition | Action |
|----------|------------|--------|
| **P0: Blocker** | The actuator fails to stop at the force threshold or on an interlock; the E-stop does not cut the switched supply; the PWA fails to load offline. | Cannot demonstrate or deploy. Isolate the hardware loop first. |
| **P1: High** | A step is skipped or repeated in the cycle; readings are NaN or stale for over 1 second; the batch record is not written. | Fix before the evaluation runs. |
| **P2: Medium** | Water volume or force differs from the reference by more than the calibration margin. | Recalibrate; document the margin. |
| **P3: Low** | Text wraps badly on small phone screens. | Backlog. |

---

## 6. Release Criteria (Definition of Done: October 2026 Gate)

The prototype is approved for demonstration and evaluation when all conditions are met:
- [ ] All P0 bugs resolved.
- [ ] Happy paths H-01 to H-06 pass.
- [ ] Sad paths S-01, S-02, S-04 and S-05 pass (the safety-relevant ones).
- [ ] Force threshold and flow volume are calibrated and recorded.
- [ ] PWA loads offline with no external requests.
- [ ] Batch CSV rows append to the SD card without filesystem errors.

---

## 7. AI / LLM Evaluation

N/A: no AI integration in this release.

---

## Self-Check

- [x] Every Must-Have PRD feature has >= 1 Happy Path
- [x] Every Happy Path has corresponding Sad Path coverage where risk exists
- [x] Automated checks reflect what exists (builds); unit tests are marked planned
- [x] Section 7 marked N/A
- [x] Release criteria are binary pass/fail
- [x] Test data setup documented

---

*Next in sequence: [CLR](CLR_Template.md): compliance launch gate.*
