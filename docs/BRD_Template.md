# Business Requirements Document (BRD)

**Project:** AUTOBRIQ: Fully Automated Paper-Charcoal Briquette Machine
**Date:** 2026-10-19
**Version:** 1.1.0
**Owner:** PUP Computer Engineering Group 6 (Añonuevo, Berongoy, Galdiano, Tiedra)
**Status:** In Review
**Source of truth:** [chapters/AUTOBRIQ_fully_automated_thesis.md](../chapters/AUTOBRIQ_fully_automated_thesis.md) · build status in [PROJECT_CONTEXT.md](PROJECT_CONTEXT.md)

---

## 1. Executive Summary

The target customers are rural households and small-scale fuel producers in Cavinti, Laguna, Philippines (study areas: Barangay Anglas and Barangay Poblacion), who rely on solid fuels as Liquefied Petroleum Gas (LPG) prices rise. Making paper-charcoal briquettes from waste paper and charcoal fines is a recognized substitute, but the manual process is labor-intensive, slow and physically demanding, and manual presses give uncontrolled, unrepeatable density. AUTOBRIQ is a solar-powered, **fully automated** cyber-physical machine: after the operator selects the water level, loads shredded paper and crushed charcoal, inserts the mold and presses START, the machine runs water dispensing, soaking and mixing in a shared container, gravity discharge into a partitioned mold, compression molding and press retraction by itself. The cycle ends there: **drying is not part of the machine**, and the briquettes are air-dried outside it so the machine is free for the next batch sooner. The immediate objective is a working off-grid prototype with an on-machine touchscreen and an offline Progressive Web App (PWA), evaluated against the manual process in Cavinti. The longer-term objective is community-run briquette production that diverts paper waste and supplies affordable cooking fuel.

---

## 2. The Problem & Opportunity

**The Problem:**
Rising LPG prices push households toward charcoal, and wood charcoal drives environmental pressure on forests. Paper-charcoal briquettes can replace part of it, but the production technology, not the slurry chemistry, is the main barrier to access. The current choices are two extremes: capital-intensive industrial plants that need three-phase grid power (unusable off-grid), or cheap manual presses that cannot hold controlled, repeatable parameters, giving unpredictable fuel density. The manual workflow needs repeated human effort for grinding, soaking, mixing, molding and compression, which means high man-hour consumption and inconsistent output.

**The Opportunity:**
A gap exists for a small, affordable, self-contained machine. Low-power microcontrollers (ESP32), a flow sensor and a load cell can replace human guesswork with deterministic, sensor-driven control of the water ratio and the compaction limit. A 110 W solar panel feeding an EcoFlow RIVER 2 power station makes the machine deployable off-grid. An offline PWA served from the microcontroller's own flash lets operators and local officials watch a batch and download CSV logs from a phone without internet.

**Target Customer / User:**
*   **Primary User (Community Operator):** Local residents or cooperative workers in Cavinti, with limited technical training. They need simple, step-by-step prompts, large controls, and clear audio-visual fault signals. Their involvement is limited to: select the water level, load the paper and crushed charcoal when prompted, insert the mold, start the batch, and remove the mold at the end.
*   **Secondary Buyer/Stakeholder (LGU / NGO Officers):** Local Government Unit (LGU) waste managers and non-governmental organizations who fund deployments to meet waste-reduction targets. They care about CSV batch logs, output yield and cost.

---

## 3. Strategic Alignment

This project aligns with regional and global sustainability goals:
1.  **UN SDG 7 (Affordable and Clean Energy):** An affordable alternative cooking fuel, produced with a solar-powered machine that works where grid power is limited.
2.  **UN SDG 12 (Responsible Consumption and Production):** Turning discarded paper and charcoal fines into fuel supports a circular economy.
3.  **UN SDG 13 (Climate Action):** Reduces demand for freshly cut wood charcoal.
4.  **Local Waste Diversion:** Helps the Municipality of Cavinti intercept paper and biomass waste before landfill.
5.  **DENR Forestry Protection:** Offsets demand for wood charcoal, which the DENR links to illegal tree cutting.

---

## 4. Scope

**In Scope:**
*   **Automated Production Cycle:** Embedded control (ESP32 finite state machine) of five phases: initialization, loading and batch start; water dispensing and mixing; mixture discharge; leveling and compression; retraction, unloading and batch completion.
*   **Sensor Feedback:** YF-S201 flow sensor (water volume), 300 kg S-type load cell with HX711 (compression force), three limit switches (trapdoor closed, compressor home, mold seated), safety interlocks and an emergency stop.
*   **Dual Interfaces:** On-machine Nextion 7" touchscreen (batch start, status, readings, faults) and an offline React PWA hosted over the ESP32's SoftAP (monitoring, batch status, CSV downloads).
*   **Local Data Logging:** One CSV row per batch on a FAT32 Micro-SD card (runtime, selected water level, dispensed water volume, peak compaction force).
*   **Solar Power:** 110 W panel, EcoFlow RIVER 2 power station, 20 V mixer supply.
*   **Output:** A solid rectangular-prism briquette in a single fixed size, **at least 20 briquettes per batch** from a partitioned mold.

**Out of Scope:**
*   **Thermal drying:** There is no drying subsystem. Water removal inside the machine is mechanical only; briquettes are air-dried outside the machine before fuel testing or use.
*   **Material handling:** Loading paper, charcoal and water selection, inserting the mold, and pulling out the filled mold stay manual.
*   **Impurity removal:** The machine cannot extract water-insoluble impurities from waste paper.
*   **Variable output:** One briquette size and shape only.
*   **Relational database:** The ESP32 cannot run SQL; logging is flat CSV.
*   **Cellular / cloud connectivity:** Strictly a local edge node over SoftAP.

---

## 5. Success Metrics

The study compares the machine with the manual process and tests the output. Numeric targets are set from the measured manual baseline and from calibration, not assumed in advance.

| Metric | Baseline | Target | Timeline |
|--------|----------|--------|----------|
| **Man-Hour Consumption** | Measured from manual production trials (15 end-user participants, stopwatch and observation logs) | Percentage reduction vs the manual mean, equal quantities compared | Pilot testing |
| **Throughput Time** | Mean manual time per batch, same trials | Percentage reduction vs the manual mean | Pilot testing |
| **Production Rate** | Manual units per hour | Higher units/hr (briquettes produced / operating hours) | Pilot testing |
| **Batch Yield** | n/a | At least 20 briquettes per batch | Every batch |
| **Briquette Consistency** | n/a | Low standard deviation of mass and dimensions over 30 specimens | Post-production testing |
| **Structural Durability** | Store-bought charcoal and manual briquettes as controls | Shatter-resistance index from a 1 m drop test | Post-production testing |
| **Fuel Performance** | Store-bought charcoal control | Burning duration and heat retention (infrared thermometer) at least match the control | Post-production testing |
| **Compression Force** | Inconsistent manual force | Preset threshold set by calibration, within the 1,500 N actuator limit | Calibration phase |
| **System Acceptability** | n/a | ISO 25010 weighted mean (functional suitability, performance efficiency, usability, reliability) | After guided runs |
| **Dashboard Reachability** | No monitoring | PWA loads fully offline from the ESP32 | Initial boot test |

---

## 6. Stakeholders & Owners

| Role | Person / Entity | Responsibility |
|------|-----------------|----------------|
| Sponsor / Decision Maker | PUP Faculty (adviser and panel); Cavinti Municipal LGU as pilot partner | Project approval, evaluation, pilot site |
| Business Owners | Community Cooperative Lead | Operations oversight, material coordination, safety assurance |
| Product / Tech Leads | Añonuevo, Berongoy, Galdiano, Tiedra | Cyber-physical development, software architecture, calibration, testing |

---

## Self-Check

- [x] Section 1 reads cleanly for a non-technical stakeholder and conveys business value
- [x] Section 2 describes the problem and the gap it fills
- [x] Section 5 has metrics tied to the study's measurements; targets not yet measured are marked as such
- [x] Section 4 explicitly names out-of-scope items (no drying, manual material handling)
- [x] No section describes *how* to build the solution (architecture lives in [SDD_Template.md](SDD_Template.md))

---

*Next in sequence: [PRD](PRD_Template.md) → [DSD](DSD_Template.md) → [SDD](SDD_Template.md).*
