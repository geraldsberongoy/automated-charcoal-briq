# Go-To-Market (GTM) Strategy

**Project:** AUTOBRIQ Fully Automated Paper-Charcoal Briquette Machine
**Date:** 2026-10-19
**Version:** 1.1.0
**Owner:** PUP Computer Engineering Group 6 (Añonuevo, Berongoy, Galdiano, Tiedra)
**PRD:** [PRD_Template.md](PRD_Template.md)

---

> **Scope:** This strategy covers the evaluation and pilot in Barangay Anglas and Barangay Poblacion, Cavinti, Laguna, Philippines, and later expansion to neighboring community-based waste management cooperatives. Public launch is hard-gated by the compliance checklist in [CLR_Template.md](CLR_Template.md).
>
> **Claims rule:** Performance claims below are targets. Replace each with the measured value from the study (man-hours, throughput, yield, burning duration, heat retention, drop test) before it is used in public material.

---

## 1. Product Summary (GTM View)

**What it does (one sentence):**
AUTOBRIQ is a solar-powered, fully automated machine that turns waste paper and residual charcoal fines into rectangular, extended-burn cooking briquettes: after the operator loads the materials and presses START, it dispenses the water, mixes, discharges into a partitioned mold and compresses by itself, and a touchscreen and an offline phone dashboard show the progress.

**Who it's for:**
Local waste cooperatives, off-grid barangay communities, and small-scale street food vendors in Cavinti, Laguna, who rely on wood charcoal and want lower fuel cost and less physical labor.

**Core value proposition:**
Communities make their own cooking fuel from waste, with controlled and repeatable water ratio and compaction instead of hand pressing, and far less labor per batch (the study measures how much). The machine cycle stops after compression so it is free for the next batch sooner; briquettes air-dry outside it.

**Category:**
Waste-to-Energy Cyber-Physical Systems (Edge IoT).

---

## 2. Target Audience

**Primary Ideal Customer Profile (ICP):**
*   **Who:** Community cooperative operators and street food vendors (for example barbecue stands) in Cavinti, Laguna. They spend a significant part of their day budget on cooking fuel and have low-to-moderate digital literacy.
*   **Where they hang out:** Cooperative centers, public markets, barangay council assemblies.
*   **What they already believe:** "Paper briquettes are weak, burn too fast, or are too exhausting to make by hand."
*   **Activation hook:** A live demonstration of a machine-pressed rectangular briquette that survives the standard 1 m drop test and burns as long as, or longer than, store-bought charcoal in a side-by-side test.

**Secondary Audience:**
*   **Who:** Local government unit (LGU) environmental officers and Solid Waste Management (SWM) administrators.
*   **Why secondary:** They fund and deploy machines to meet municipal waste-diversion targets.

---

## 3. Pricing Model

**Model:** Cooperative Livelihood / Social Franchise Model. The machine is sponsored or grant-funded by LGUs, while the briquettes are sold by the cooperative.

| Product/Tier | Target Price | Details | Conversion / Value Gate |
|------|-------|-----------------|--------------|
| **Community Machine Setup** | Sponsored / LGU Grant | Complete machine with solar hardware, Nextion screen and PWA dashboard. | LGU grant approval and pilot site clearance. |
| **Briquette Output (per kg)**| 15.00 PHP (draft) | Rectangular paper-charcoal briquettes. | Priced below traditional wood charcoal. |

**Pricing rationale:**
Wood charcoal in Laguna public markets was taken as about 25.00 PHP per kg in the earlier draft. Raw materials (waste paper and charcoal fines) are sourced at zero or near-zero cost. These price points were carried over from the earlier draft of this document and are not in the thesis: re-check them against current market prices before use.

---

## 4. Positioning & Messaging

**Tagline:** `Powering Waste-to-Energy Cooperatives, One Smart Compaction at a Time.`

**Primary message (hero):**
"Turn local paper waste and charcoal fines into long-burn briquettes. Load the materials, press START, and AUTOBRIQ does the water, the mixing and the pressing for you, on solar power."

**Proof points:**
*   **Safety Guided:** Compression force is monitored by a load cell and limit-switch interlocks watch the trapdoor and the mold, with an emergency stop that cuts the machine's switched 12 V supply.
*   **Solar Powered:** Runs off-grid from a 110 W solar panel and an EcoFlow RIVER 2 power station.
*   **Offline Dashboard:** Join the machine's own Wi-Fi to watch the batch and download the batch log on any phone, no internet needed.

---

## 5. Launch Channels & Tactics

**Community Channels:**

| Channel | Target Audience | Planned Action |
|---------|----------|----------------|
| **Barangay Assemblies** | Co-op members & residents | Live on-site demonstration: loading, START, the automatic mixing and pressing, then briquettes that dry outside the machine. |
| **LGU Environmental Seminars** | Waste managers & LGU leads | Present the CSV batch logs showing waste throughput and batch results. |

**Earned Channels:**
*   **Demonstration Videos:** A short instructional video showing how to join `Operator-Node-01` and start a batch.

---

## 6. Launch Phases

| Phase | Criteria to Enter | Target Date | Goal |
|-------|-------------------|-------------|------|
| **Phase 1: Prototype Validation** | QAD release criteria met; zero P0 safety faults. | October 2026 | Working prototype shown at the defense. |
| **Phase 2: Guided Evaluation** | Machine calibrated; consent obtained. | After the defense (date to be set) | Guided runs with 15 end-users in Barangay Anglas and Barangay Poblacion; 30 specimens tested; ISO 25010 survey. |
| **Phase 3: Wider Deployment** | CLR gates cleared; study results in; LGU funding secured. | To be set | Additional machines in other Cavinti barangays. |

> **Hard gate:** Public expansion cannot proceed while any safety interlock fault or data-logging failure is unresolved.

---

## 7. Success Metrics (post-evaluation)

| Metric | Target | How to Measure |
|--------|--------|----------------|
| **Batch Yield** | At least 20 briquettes per batch | Batch counter |
| **Labor and Time Saved** | Reduction in man-hours and throughput time vs the manual baseline | Stopwatch and observation logs (percentage-reduction formulas) |
| **Fuel Performance** | Burning duration and heat retention at least match store-bought charcoal | Stopwatch and infrared thermometer, same conditions |
| **Structural Durability** | Low shatter-resistance index | 1 m drop test on the 30 specimens |
| **Operator Acceptability** | High ISO 25010 weighted mean (functional suitability, performance efficiency, usability, reliability) | Questionnaire, 15 end-users |
| **Dashboard Reachability** | Phone connects and loads the PWA in every session | Client connection checks |

---

## Self-Check

- [x] Section 2 ICP is specific enough to name real people or grounded personas
- [x] Section 3 pricing is marked as a draft from the marketing plan and flagged for re-check
- [x] Section 5 content assets are listed and tied to launch
- [x] Section 6 phase transitions are binary; expansion is hard-gated on the CLR
- [x] Section 7 metrics match what the thesis measures
- [x] Claims are targets until replaced by measured values

---

*Document suite complete. Sequence: [BRD](BRD_Template.md) → [PRD](PRD_Template.md) → [DSD](DSD_Template.md) → [SDD](SDD_Template.md) → [QAD](QAD_Template.md) → [CLR](CLR_Template.md) → GTM.*
