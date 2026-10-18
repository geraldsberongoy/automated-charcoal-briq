# Compliance & Legal Readiness Register (CLR)

**Project:** AUTOBRIQ Fully Automated Paper-Charcoal Briquette Machine
**Date:** 2026-10-19
**Version:** 1.1.0
**Owner:** PUP Computer Engineering Group 6 (Añonuevo, Berongoy, Galdiano, Tiedra)
**PRD:** [PRD_Template.md](PRD_Template.md)
**SDD:** [SDD_Template.md](SDD_Template.md)

---

> ⚠️ **Structural and regulatory awareness only. NOT legal advice.** This register maps the data processed by the machine, the PWA and the research study, and surfaces obligations. It does not draft the Privacy Policy or Terms of Use and does not replace licensed counsel.

---

> **Demo vs. commercial boundary:** The machine runs entirely offline over a local SoftAP with no WAN/internet connection. It collects basic device metadata (temporary client MAC addresses for DHCP routing) and production batch statistics (selected water level, dispensed water volume, runtime, peak compaction force, status). The **research evaluation** with end-users in Barangay Anglas and Barangay Poblacion, Cavinti, Laguna additionally collects informed-consent records, observation logs from the manual and machine runs, and ISO 25010 questionnaire responses from 15 participants. Machine data persists locally on the Micro-SD card and does not transit cross-border.

---

## 0. Target Markets

| Region | In scope? | Notes |
|--------|-----------|-------|
| Philippines | **Yes** | Primary market and study site (Republic Act No. 10173, Data Privacy Act of 2012). |
| Other regions | No | Out of scope. |

**Geo-blocking:**
The SoftAP broadcasts locally within about 15 meters and has no WAN path, so it cannot be reached from the internet.

---

## 1. Data Inventory / Record of Processing

| Activity | Purpose | Data categories | Data subjects | Recipients / sub-processors | Cross-border transfer | Retention | Legal basis |
|----------|---------|-----------------|---------------|-----------------------------|-----------------------|-----------|-------------|
| **Wi-Fi Client Connection** | DHCP routing to serve PWA pages. | Client MAC address | Connected operators | None (RAM only) | No | Volatile (deleted on disconnect) | Contract (necessary to serve the app) |
| **Batch Production Logging** | Yield tracking and diagnostics. | Batch ID, water level, dispensed volume, runtime, peak force, status, system event logs | Operators (indirectly) | None (persists on the local SD card) | No | 3 years (LGU waste audit requirement) | Legitimate Interest (operational metrics) |
| **Research Participation** | Time-and-motion baseline and guided machine runs. | Informed-consent record, observation and timing logs | 15 end-users from Barangay Anglas and Barangay Poblacion | Researchers only | No | For the study period; group and adviser to confirm | Consent |
| **Acceptability Survey** | ISO 25010 questionnaire on the system and its interface. | Questionnaire (Likert) responses | The same 15 end-users | Researchers only | No | For the study period; group and adviser to confirm | Consent |

**Sensitivity flags:**

| Data type | Collected? | Notes |
|-----------|-----------|-------|
| Basic PII (name, email) | Study only | Consent forms identify participants; the machine itself requires no login. Responses should be tallied without names. |
| Special-category / sensitive | No | |
| Children's data (minors) | No | Participants and operators are adults. |
| Precise location | No | Study sites are named at barangay level only. |
| Photos / camera / microphone | No | |
| Device IDs / advertising IDs | No | |
| Analytics / telemetry | **Yes** | Local device uptime and connection statistics. |
| Crash logs | **Yes** | Written to the local SD card. |
| Payment / card data | No | |

**Self-check:**

| Item | Done? | Evidence link | Counsel needed? |
|------|-------|---------------|-----------------|
| Every processing activity has a retention period | Partly | Study retention to be confirmed with the adviser | No |
| Every sub-processor is named and has a DPA in place | Yes | N/A (no sub-processors used) | No |
| Inventory is dated and treated as living | Yes | Document header | No |

---

## 2. Multi-Jurisdiction Obligations Matrix

| Dimension | Philippines DPA 2012 (RA 10173) |
|-----------|----------------------------------|
| **Consent / legal basis** | Machine: entering the Wi-Fi password to join `Operator-Node-01` confirms local use. Study: written informed consent from each participant before any testing, as the data-gathering procedure states. |
| **Data subject rights** | Operators can have the SD card formatted to delete run records. Recommended for the study: let participants withdraw and have their responses removed before analysis (not stated in the thesis; confirm with the adviser). |
| **Breach notification** | The machine is local-only and a breach needs physical access. If an SD card is stolen the LGU is notified. Recommended: keep consent forms and completed questionnaires under the researchers' control until tallied. |
| **DPO / accountability** | LGU cooperative lead as local data handler for machine data; the research group for study data. |
| **Our status / action** | Posture is secure because there is no internet access. SD card data is unencrypted, so the machine enclosure must be lockable. Set the study retention period with the adviser. |

---

## 3. Escalation Flags: Counsel Required

| Flag | Present? | Why it escalates |
|------|----------|------------------|
| Children's data | No | |
| Health / medical data | No | |
| Payments / card data | No | |
| Biometric data | No | |
| Large-scale / systematic monitoring | No | |
| **Automated decisions with legal/significant effect** | **Yes** | The machine automatically stops its production loads on an interlock or force fault. Mitigation: the physical emergency stop cuts the switched 12 V supply in hardware, independent of the firmware. |
| Sale / share / behavioral advertising | No | |
| Operating in a market with no local entity | No | |

**DPIA / PIA required?** No: no sensitive data, small voluntary study group, local-only machine data.

---

## 4. Terms of Use / EULA Readiness

*Presence-check only; counsel drafts the text.*

| Clause | Present? | Evidence link | Counsel needed? |
|--------|----------|---------------|-----------------|
| License grant + scope | Yes | EULA checklist | No |
| Acceptable use / prohibited conduct | Yes | Prohibits overfilling the mixing container or the mold and bypassing the interlocks | No |
| **Limitation of liability + warranty disclaimer** | **Yes** | Disclaims liability for physical injury from the moving actuators, the 20 V mixer and the pinch points at the mold | **Yes** |
| Governing law + jurisdiction | Yes | Laguna, Philippines | Yes |
| Dispute resolution | Yes | | Yes |
| Termination + suspension | Yes | | No |
| UGC license + takedown | No | N/A (no user-generated content) | No |
| Modification / notice mechanism | Yes | | No |
| Payment / refund terms | No | N/A (free community software) | No |
| Age eligibility (minor handling) | Yes | Restricted to 18+ trained operators | Yes |
| Privacy Policy incorporated by reference | Yes | | No |

---

## 5. IP Infringement & Protection Readiness

| Item | Status | Evidence link | Counsel needed? |
|------|--------|---------------|-----------------|
| Brand-name trademark search | **Open for "AUTOBRIQ"** | The earlier search was for "Operator Node" (the dashboard name); no search has been recorded for AUTOBRIQ | Check IPOPHL before any public use |
| Open-source license compliance | Complete | MIT-licensed React, Vite and Workbox; LGPL/Apache ESP32 libraries | No |
| Copyleft scan | Clean | No GPLv3 code in firmware or client | No |
| Third-party assets licensed | Clean | Icons are local; no CDN-loaded fonts or assets | No |
| AI vendor output ownership | Clean | N/A (no AI model used) | No |
| Written IP assignment | Complete | Group 6 joint assignment of prototype IP to the LGU partner | Yes |

---

## 6. Platform Compliance

N/A: served as an offline browser page over a local SoftAP. No Apple App Store or Google Play deployment is planned.

---

## Self-Check

- [x] Section 0 declares every market; demo-vs-commercial reachability is stated honestly
- [x] Section 1 has one row per processing activity, including the research study
- [x] Section 2 filled for the Philippines jurisdiction
- [x] Section 3 "Yes" mitigation is stated; disclaimer banner is at the top
- [x] Section 4 ToU clauses presence-checked (drafting left to counsel)
- [x] Section 5 flags the open AUTOBRIQ trademark search
- [x] Section 6 marked N/A
- [x] This document maps obligations and escalates; it does not give legal advice

---

*Document suite complete. Sequence: [BRD](BRD_Template.md) → [PRD](PRD_Template.md) → [DSD](DSD_Template.md) → [SDD](SDD_Template.md) → [QAD](QAD_Template.md) → [CLR](CLR_Template.md) → [GTM](GTM_Template.md).*
