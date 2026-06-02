# Business Requirements Document (BRD)

**Project:** Semi-Automated Paper-Charcoal Briquette Machine
**Date:** 2026-06-03
**Version:** 1.0.0
**Owner:** PUP Computer Engineering Group 6 (Añonuevo, Berongoy, Galdiano, Tiedra)
**Status:** In Review

---

## 1. Executive Summary

The target customers are rural households and small-scale fuel manufacturers in Cavinti, Laguna, Philippines, who rely on traditional solid fuels due to the volatile cost and accessibility of Liquefied Petroleum Gas (LPG) and grid electricity. The manual production of paper-charcoal briquettes is currently bottlenecked by intensive physical labor, slow throughput, and inconsistent product quality. The Semi-Automated Paper-Charcoal Briquette Machine is an off-grid, solar-powered cyber-physical system that automates the sequential processing of waste paper, water, and charcoal dust into uniform, high-density fuel briquettes. The immediate business objective is to deploy a functional, off-grid prototype with a local Human-Machine Interface (HMI) and a responsive Progressive Web App (PWA) to serve as a community waste-to-energy pilot in Cavinti. The longer-term commercial objective is to establish community-run solid waste management enterprises that reduce landfill waste and supply affordable cooking fuel, tapping into a localized alternative energy market valued for waste diversion and carbon offset potentials.

---

## 2. The Problem & Opportunity

**The Problem:**
Traditional solid fuel sourcing in rural Philippine communities relies heavily on freshly cut wood charcoal, driving environmental degradation, illegal logging, and local timber poaching. While paper-charcoal briquetting (using waste paper as a binder and charcoal fines as combustible fuel) is a recognized substitute, the manual process is highly inefficient. It demands constant, exhausting manual labor for grinding, mixing, and pressing, resulting in high man-hour consumption. Additionally, manual compaction leads to severe inconsistencies in briquette density, moisture retention, and combustion duration, leading to unpredictable fuel performance. Commercial industrial plants resolve this but cost tens of thousands of dollars and require high-voltage, three-phase grid power, which is unavailable in off-grid rural communities.

**The Opportunity:**
A critical whitespace exists for a mid-scale, affordable, and self-contained cyber-physical production system. By leveraging cheap, low-power microcontrollers (ESP32) and closed-loop sensory feedback, we can replace human guesswork with deterministic control logic. The integration of solar power (100W solar panel and 30Ah LiFePO4 battery bank) enables deployability in remote, off-grid areas. Introducing a responsive PWA dashboard served directly from the microcontroller's internal storage allows local community leaders and technicians to monitor telemetry, evaluate machine health, and access production logs without relying on internet access or external networks.

**Target Customer / User:**
*   **Primary User (Community Operator):** Local residents or cooperative workers in Cavinti, Laguna, who operate the machine daily. They have limited technical training, require simple, step-by-step guidance, and need high visual/audio safety indicators to handle moving parts safely.
*   **Secondary Buyer/Stakeholder (LGU / NGO Officers):** Local Government Unit (LGU) waste managers or non-governmental organizations who finance the machine deployment to meet waste-reduction targets and provide social livelihood programs. They care about CSV data logs, output yield, and cost efficiency.

---

## 3. Strategic Alignment

This project directly aligns with regional and global sustainability goals:
1.  **UN SDG 7 (Affordable and Clean Energy):** Providing an affordable alternative cooking fuel for off-grid communities.
2.  **UN SDG 12 (Responsible Consumption and Production):** Utilizing discarded waste paper and charcoal residues to promote a circular economy.
3.  **Local Waste Diversion Goals:** Assisting the Municipality of Cavinti in intercepting paper and biomass wastes prior to landfill dumping, reducing recurring hauling costs.
4.  **DENR Forestry Protection:** Offsetting the demand for forest wood charcoal, indirectly curbing illegal tree-cutting and poaching.

---

## 4. Scope

**In Scope:**
*   **Sequential FSM Control:** Embedded software to run a 5-stage production flow (Initialization, Grinding/Soaking, Combining, Compression/Molding, and Thermal Curing).
*   **Sensory Feedback Network:** Integration of a YF-S201 flow meter, HX711 compaction load cell (target 2000N), limit switches for safety gates, DS18B20 temperature probes, and DHT22 humidity sensors.
*   **Dual HMIs:** On-board 4.3" Nextion touchscreen for step-by-step physical operation, and a local offline React PWA dashboard hosted over the ESP32's SoftAP for wireless telemetry.
*   **Local Data Logging:** Standalone CSV logging onto a FAT32 Micro-SD card.

**Out of Scope:**
*   **Fully Autonomous Feeding/Transfers:** Raw material loading, water loading, and physical transfer of pressed briquettes from mold to curing rack remain manual.
*   **Relational Database Engine:** The ESP32 does not support SQL; all local logging is flat CSV.
*   **Cellular / Cloud Connectivity:** Operations are strictly local area network (edge mode) to maintain off-grid efficiency.

---

## 5. Success Metrics

| Metric | Baseline | Target | Timeline |
|--------|----------|--------|----------|
| **Man-Hour Consumption** | 5.2 hours per 30 manual briquettes | < 1.0 hour per 30 briquettes | Pilot deployment (Phase 1) |
| **Briquette Compaction Consistency** | Variance of +/- 35% in density | Variance of < 5% in density | Post-testing verification |
| **Volumetric Inflow Accuracy** | Manual estimates (highly variable) | 1:1 water-to-paper ratio (+/- 2% margin) | Calibration phase |
| **Compaction Force** | Inconsistent human force | Sustained 2000N compaction load | Live run verification |
| **Local Dashboard Accessibility** | No monitoring capability | 100% offline uptime of PWA | Initial boot test |

---

## 6. Stakeholders & Owners

| Role | Person / Entity | Responsibility |
|------|-----------------|----------------|
| Sponsor / Decision Maker | Cavinti Municipal LGU & PUP Faculty | Final project approval, funding, pilot site provision |
| Business Owners | Community Cooperative Lead | Operations oversight, material coordination, safety assurance |
| Product / Tech Leads | Añonuevo, Berongoy, Galdiano, Tiedra | Cyber-physical development, software architecture, calibration |

---

## Self-Check

- [x] Section 1 reads cleanly for a non-technical stakeholder and conveys business value
- [x] Section 2 quantifies the problem where possible
- [x] Section 5 has metrics with numbers and timelines
- [x] Section 4 explicitly names out-of-scope items
- [x] No section describes *how* to build the solution (architecture lives in [sdd-automated-charcoal-briq.md](../docs/SDD_Template.md))

---

*Next in sequence: [PRD](PRD_Template.md) → [DSD](DSD_Template.md) → [SDD](SDD_Template.md).*
