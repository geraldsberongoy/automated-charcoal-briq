# QA & Test Plan (QAD)

**Project:** Semi-Automated Paper-Charcoal Briquette Machine Dashboard
**Date:** 2026-06-03
**Version:** 1.0.0
**Owner:** PUP Computer Engineering Group 6 (Añonuevo, Berongoy, Galdiano, Tiedra)
**PRD:** [PRD_Template.md](PRD_Template.md)

---

> **Context:** "Launch" for this plan equals the **functional pilot deployment and presentation gate (May 2026)**. A passing result means the cyber-physical machine safely operates, registers the local PWA service worker, displays sensor telemetry, and halts on safety breaches.

---

## 1. Testing Strategy & Scope

**In Scope:**
*   **Offline Access & Service Worker Registration:** Validating that the PWA registers `/sw.js` and remains fully loadable when the mobile device has no cellular or internet access.
*   **FSM Transitions:** Validating stage-by-stage command execution via the PWA (Initialization -> Grinding -> Combining -> Compaction -> Curing -> Curing Complete).
*   **Compaction Threshold Halt:** Verifying the HX711 sensor triggers FSM actuation shutdown when compaction force hits 2000N.
*   **Interrupt-level Safety Override:** Testing physical limit switches to ensure actuator stops immediately on casing breach detection.
*   **Enclosure Curing Termination:** Checking the system terminates the halogen array relay when humidity drops to or below 15%.

**Out of Scope (this gate):**
*   **Multi-machine cluster coordination:** Monitoring multiple machines simultaneously.
*   **Enterprise Security Compliance:** Pentesting or WAN vulnerability audits, as the SoftAP operates in a local range of 15m without WAN connection.

**Testing Levels:**

| Level | Tooling | Owner |
|-------|---------|-------|
| **Unit Testing** | Arduino FakeIt / local mocks | Berongoy |
| **Integration Testing**| Hardware-in-the-loop (HIL) bench setup | Añonuevo |
| **System E2E Testing** | Manual test cycles on fully assembled machine | Group 6 |

---

## 2. Test Environments & Data

*   **Physical Staging:** Assembled machine prototype powered by a bench power supply simulating LiFePO4 battery discharge curve (11.0V to 14.2V).
*   **Dashboard Staging:** Chrome DevTools console and mobile devices (Android 12+, iOS 16+) connected to SSID `Operator-Node-01` (`http://192.168.4.1`).
*   **Test Data Setup:** Sensor mock configurations in firmware:
    *   `/api/stats` mock parameters compiled under `dev` mode to test edge cases (e.g. current spikes, sensor disconnects).

---

## 3. Core Test Scenarios

### Happy Paths

| ID | Scenario | Steps | Expected Result | US-ID |
|----|----------|-------|-----------------|-------|
| **H-01** | Offline Dashboard Launch | 1. Turn on ESP32 SoftAP.<br>2. Disable mobile data on phone.<br>3. Connect to `Operator-Node-01`.<br>4. Open `http://192.168.4.1`. | The dashboard loads instantly, and the Chrome console logs `/sw.js registered successfully`. | US-01 |
| **H-02** | Automated Water Solenoid Cutoff | 1. Initialize Grinding/Soaking Phase.<br>2. Solenoid opens (water flows).<br>3. Inject YF-S201 pulses to simulate flow. | Solenoid valve closes immediately when calculated flow reaches the target volume. | US-01 |
| **H-03** | Auto Compaction Stop | 1. Begin Compaction Phase.<br>2. Actuator drives down.<br>3. Apply load on load cell (HX711). | The actuator halts moving when force reaches 2000N, and the PWA transitions to the next step. | US-02 |
| **H-04** | Auto Curing Cutoff | 1. Begin Thermal Curing Phase.<br>2. Relays open (halogens and fan turn on).<br>3. Drop local humidity reading below 15%. | Curing components turn off, buzzer pulses, and PWA displays "Drying Complete". | US-04 |

### Sad Paths

| ID | Scenario | Input / Trigger | Expected Behavior |
|----|----------|-----------------|-------------------|
| **S-01** | Casing Breach Halt | Open the safety casing limit switch during compaction. | The FSM drops actuator duty cycle to 0% in under 5 ms, buzzer alerts sound, and PWA flashes a critical red warning box. |
| **S-02** | Sensor Disconnection | Unplug the HX711 or DHT22 data line. | Dashboard shows a yellow warning badge next to the affected gauge. FSM prevents transitioning into the affected phase. |
| **S-03** | Memory Exceeded | Simulate 5 clients continuously querying the API. | ESP32 heap allocations remain stable. The web server drops requests exceeding connection capacity without restarting. |

---

## 4. Automation vs. Manual Testing

### Automated CI Gate
```yaml
# PlatformIO CLI Test Stage
- run: pio test -e esp32doit-devkit-v1
- run: npm run test (dashboard unit tests)
- GATE: unit tests pass with 0 failures
```

### Manual Verification
*   **Chassis Safety Inspection:** Manual trigger of safety limit switches before every operational shift.
*   **Sensor Zero-Point Calibration:** Running scale calibration on the S-Type load cell and HX711 module.
*   **Mobile Browser Spot Checks:** Opening the PWA dashboard on Chrome (Android) and Safari (iOS) under direct sunlight to check visibility and button response times.

---

## 5. Bug Triage Protocol

| Severity | Definition | Action |
|----------|------------|--------|
| **P0 — Blocker** | Actuator fails to stop on interlock open or 2000N overload; PWA fails to load offline. | Cannot deploy. Revert code and isolate hardware loops. |
| **P1 — High** | Telemetry lag exceeds 1 second; sensor values display corrupted NaN characters. | Fix before deployment. |
| **P2 — Medium** | Solar charge metrics show +/- 10% drift compared to MPPT output. | Document drift; deploy with calibration notes. |
| **P3 — Low** | Event log text styling wraps improperly on small mobile screens. | Log in backlog for post-launch cleanup. |

---

## 6. Release Criteria (Definition of Done — May 2026 Pilot Gate)

Deployment is approved when all conditions are met:
- [ ] All P0 bugs resolved.
- [ ] Happy paths H-01 through H-04 pass.
- [ ] Interlock safety interrupt is verified to halt the linear compactor actuator within 5 ms.
- [ ] PWA offline capability passes Lighthouse check without connection.
- [ ] Standalone SD card logs compile and append without filesyste errors.

---

## 7. AI / LLM Evaluation

N/A — no AI integration in this release.

---

## Self-Check

- [x] Every Must-Have PRD feature has >= 1 Happy Path
- [x] Every Happy Path has corresponding Sad Path coverage where risk exists
- [x] Automated checks defined for the compile/build gate
- [x] Section 7 marked N/A
- [x] Release criteria are binary pass/fail
- [x] Test data setup documented

---

*Next in sequence: [CLR](CLR_Template.md) — compliance launch gate.*
