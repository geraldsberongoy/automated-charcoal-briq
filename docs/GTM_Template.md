# Go-To-Market (GTM) Strategy

**Project:** Semi-Automated Paper-Charcoal Briquette Machine Dashboard
**Date:** 2026-06-03
**Version:** 1.0.0
**Owner:** PUP Computer Engineering Group 6 (Añonuevo, Berongoy, Galdiano, Tiedra)
**PRD:** [PRD_Template.md](PRD_Template.md)

---

> **Scope:** This strategy covers the pilot rollout in Cavinti, Laguna, Philippines, and the subsequent expansion to neighboring community-based waste management cooperatives. Public launch is hard-gated by the compliance checklist in [CLR_Template.md](CLR_Template.md).

---

## 1. Product Summary (GTM View)

**What it does (one sentence):**
The Semi-Automated Paper-Charcoal Briquette Machine converts paper waste and residual charcoal fines into high-density, extended-burn cooking briquettes using a solar-powered, sensor-guided compaction system monitored wirelessly via an offline PWA dashboard.

**Who it's for:**
Local waste cooperatives, off-grid barangay communities, and small-scale street food vendors in Cavinti, Laguna, who rely on wood charcoal and seek to reduce fuel costs and physical labor.

**Core value proposition:**
Empowers communities to manufacture their own cooking fuel at 40% lower cost than store-bought wood charcoal, while reducing physical labor by 80% through semi-automated, sensor-controlled cycles.

**Category:**
Waste-to-Energy Cyber-Physical Systems (Edge IoT).

---

## 2. Target Audience

**Primary Ideal Customer Profile (ICP):**
*   **Who:** Community cooperative operators and street food vendors (e.g. barbecue stands) in Cavinti, Laguna. They spend significant daily budget on cooking fuel and have low-to-moderate digital literacy.
*   **Where they hang out:** Local cooperative centers, public markets, and barangay council assemblies.
*   **What they already believe:** "Alternative paper briquettes are weak, burn too fast, or are too exhausting to make by hand."
*   **Activation hook:** Live physical demonstration showing a 2000N compressed rectangular briquette resisting a 2-meter drop test and burning twice as long as traditional charcoal.

**Secondary Audience:**
*   **Who:** Local government unit (LGU) environmental officers and Solid Waste Management (SWM) administrators.
*   **Why secondary:** They provide the capital and funding to build and deploy the machines to achieve municipal waste-diversion targets.

---

## 3. Pricing Model

**Model:** Cooperative Livelihood / Social Franchise Model. The machine is sponsored/funded by LGUs or grants, while the final briquettes are sold by the community cooperative.

| Product/Tier | Target Price | Details | Conversion / Value Gate |
|------|-------|-----------------|--------------|
| **Community Machine Setup** | Sponsored / LGU Grant | Complete machine assembly with solar hardware, Nextion screen, and PWA dashboard. | LGU grant approval and pilot site clearance. |
| **Briquette Output (per kg)**| 15.00 PHP | High-density rectangular paper-charcoal briquettes. | Retails at 40% below traditional wood charcoal. |

**Pricing rationale:**
Traditional wood charcoal in Laguna public markets retails for approximately 25.00 PHP per kg. Pricing our briquettes at 15.00 PHP per kg provides immediate household savings while generating a sustainable profit margin for the cooperative (raw materials like waste paper and charcoal dust are sourced at zero or near-zero cost).

---

## 4. Positioning & Messaging

**Tagline:** `Powering Waste-to-Energy Cooperatives, One Smart Compaction at a Time.`

**Primary message (hero):**
"Transform local paper waste and charcoal dust into high-heat, long-burn briquettes. Our solar-powered, sensor-guided machine takes the hard physical labor out of briquette production, giving you a reliable fuel source at a fraction of the cost."

**Proof points:**
*   **Safety Guided:** 2000N compaction force monitored via load cell, with automatic hardware safety interlocks.
*   **Solar Powered:** Runs 100% off-grid on a LiFePO4 battery pack charged by a 100W panel.
*   **Offline Telemetry:** Connect directly to the machine's Wi-Fi to monitor state, temperature, and yield stats on any mobile device.

---

## 5. Launch Channels & Tactics

**Community Channels:**

| Channel | Target Audience | Planned Action |
|---------|----------|----------------|
| **Barangay Assemblies** | Co-op members & residents | Live, on-site demonstration of machine pulverizing, pressing, and drying raw wastes. |
| **LGU Environmental Seminars** | Waste managers & LGU leads | Present SD card CSV run logs showing waste throughput and system uptime. |

**Earned Channels:**
*   **Demonstration Videos:** Short, 2-minute instructional video showing the simplicity of connecting to `Operator-Node-01` SoftAP and starting a batch.

---

## 6. Launch Phases

| Phase | Criteria to Enter | Target Date | Goal |
|-------|-------------------|-------------|------|
| **Phase 1: Pilot Validation** | QAD verification pass; zero P0 safety faults. | May 2026 | Complete 30-batch test run in supervised co-op site. |
| **Phase 2: Alpha Cooperative Trial** | Local operator training completed; 100% telemetry uptime. | June 2026 | Hand over operational control to 3 selected cooperative leaders. |
| **Phase 3: Public Expansion** | CLR compliance gates cleared; LGU grant funding secured. | Q3 2026 | Deploy 3 additional machines in Cavinti barangays. |

> **Hard gate:** Public launch expansion cannot proceed while any safety interlock alert or data logging failure remains unresolved.

---

## 7. Success Metrics (30-day post-pilot-launch)

| Metric | Target | How to Measure |
|--------|--------|----------------|
| **Cooperative Adoption** | 3 active trained operators | Weekly operational checks. |
| **Waste Diversion Volume** | 500 kg of waste processed | Summing values from SD card CSV logs. |
| **Briquette Yield** | > 1,500 uniform briquettes | Counting total output batch records. |
| **Fuel Burn Duration** | > 120 minutes average | Standardized boiling tests under pilot conditions. |
| **PWA Connection Success** | 100% local SoftAP reachability | Client query checks in telemetry dumps. |

---

## Self-Check

- [x] Section 2 ICP is specific enough to name real people or grounded personas
- [x] Section 3 pricing has a clear conversion gate anchored to a real benchmark (wood charcoal market rate)
- [x] Section 5 content assets listed and tied to launch
- [x] Section 6 phase transitions are binary; Public Launch is hard-gated on CLR Section 3 where applicable
- [x] Section 7 metrics are measurable and instrumented (via CSV production logging)
- [x] Drafted pre-launch as strategy, not retrospective

---

*Document suite complete. Sequence: [BRD](BRD_Template.md) → [PRD](PRD_Template.md) → [DSD](DSD_Template.md) → [SDD](SDD_Template.md) → [QAD](QAD_Template.md) → [CLR](CLR_Template.md) → GTM.*
