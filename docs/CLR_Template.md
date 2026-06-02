# Compliance & Legal Readiness Register (CLR)

**Project:** Semi-Automated Paper-Charcoal Briquette Machine Dashboard
**Date:** 2026-06-03
**Version:** 1.0.0
**Owner:** PUP Computer Engineering Group 6 (Añonuevo, Berongoy, Galdiano, Tiedra)
**PRD:** [PRD_Template.md](PRD_Template.md)
**SDD:** [SDD_Template.md](SDD_Template.md)

---

> ⚠️ **Structural and regulatory awareness only — NOT legal advice.** This register maps the data processed by the PWA and surfaces obligations. It does not draft the Privacy Policy or Terms of Use and does not replace licensed counsel.

---

> **Demo vs. commercial boundary:** The pilot deployment in Cavinti, Laguna operates entirely offline in a local area network (SoftAP) with no WAN/internet connection. The system collects basic device metadata (temporary client MAC addresses for DHCP IP routing) and production batch run statistics (grinding/soaking times, compaction forces, climate values, and runtime status). No personal PII (names, emails, contacts) is processed. Data persists locally on the physical Micro-SD card and does not transit cross-border.

---

## 0. Target Markets

| Region | In scope? | Notes |
|--------|-----------|-------|
| Philippines | **Yes** | Primary launch market (governed by Republic Act No. 10173 - Data Privacy Act of 2012). |
| Other regions | No | Out of scope. |

**Geo-blocking:**
Because the system is an off-grid SoftAP broadcasting locally within a 15-meter physical range, it is physically geo-blocked to its location and cannot be reached by external internet traffic.

---

## 1. Data Inventory / Record of Processing

| Activity | Purpose | Data categories | Data subjects | Recipients / sub-processors | Cross-border transfer | Retention | Legal basis |
|----------|---------|-----------------|---------------|-----------------------------|-----------------------|-----------|-------------|
| **Wi-Fi Client Connection** | DHCP routing to serve PWA pages. | Client MAC address | Connected Operators | None (processed in RAM only) | No | Volatile (deleted on disconnect) | Contract (necessary to serve app) |
| **Batch Production Logging** | Local yield tracking and diagnostics. | Batch ID, timestamps, sensor readings, system error logs | Connected Operators | None (persists on local SD card) | No | 3 years (LGU waste audit requirement) | Legitimate Interest (operational metrics) |

**Sensitivity flags:**

| Data type | Collected? | Notes |
|-----------|-----------|-------|
| Basic PII (name, email) | No | No login required for offline operation. |
| Special-category / sensitive | No | |
| Children's data (minors) | No | Operators must be trained cooperative members. |
| Precise location | No | |
| Photos / camera / microphone | No | |
| Device IDs / advertising IDs | No | |
| Analytics / telemetry | **Yes** | Local device uptime and network connection statistics. |
| Crash logs | **Yes** | Stack trace dumps written to local SD card. |
| Payment / card data | No | |

**Self-check:**

| Item | Done? | Evidence link | Counsel needed? |
|------|-------|---------------|-----------------|
| Every processing activity has a retention period | Yes | This doc Section 1 | No |
| Every sub-processor is named and has a DPA in place | Yes | N/A (no sub-processors used) | No |
| Inventory is dated and treated as living | Yes | Document header | No |

---

## 2. Multi-Jurisdiction Obligations Matrix

| Dimension | Philippines DPA 2012 (RA 10173) |
|-----------|----------------------------------|
| **Consent / legal basis** | Explicit password entry to connect to SSID `Operator-Node-01` serves as confirmation of localized use. |
| **Data subject rights** | Operators can request formatting of the SD card to delete run records via the Nextion Touchscreen or PWA settings page. |
| **Breach notification** | Local network only; physical access is required for a breach. In case of theft of the SD card, the LGU must be notified. |
| **DPO / accountability** | LGU Coop Lead designated as local data handler. |
| **Our status / action** | Posture is highly secure because there is no internet access. SD card data is unencrypted; physical access to the machine chassis must be locked. |

---

## 3. Escalation Flags — Counsel Required

| Flag | Present? | Why it escalates |
|------|----------|------------------|
| Children's data | No | |
| Health / medical data | No | |
| Payments / card data | No | |
| Biometric data | No | |
| Large-scale / systematic monitoring | No | |
| **Automated decisions with legal/significant effect** | **Yes** | The machine automatically halts the compactor if sensor parameters or safety gates are open. Mitigation: A physical emergency stop button bypasses firmware to prevent safety issues from system lock. |
| Sale / share / behavioral advertising | No | |
| Operating in a market with no local entity | No | |

**DPIA / PIA required?** No, as no PII is collected or processed.

---

## 4. Terms of Use / EULA Readiness

*Presence-check only; counsel drafts the text.*

| Clause | Present? | Evidence link | Counsel needed? |
|--------|----------|---------------|-----------------|
| License grant + scope | Yes | EULA checklist | No |
| Acceptable use / prohibited conduct | Yes | Prohibits overloading compactor hopper | No |
| **Limitation of liability + warranty disclaimer** | **Yes** | Disclaims liability for physical injury from moving compactor and high-speed motors. | **Yes** |
| Governing law + jurisdiction | Yes | Laguna, Philippines | Yes |
| Dispute resolution | Yes | | Yes |
| Termination + suspension | Yes | | No |
| UGC license + takedown | No | N/A (no user generated content) | No |
| Modification / notice mechanism | Yes | | No |
| Payment / refund terms | No | N/A (free community software) | No |
| Age eligibility (minor handling) | Yes | Restricted to 18+ trained operators | Yes |
| Privacy Policy incorporated by reference | Yes | | No |

---

## 5. IP Infringement & Protection Readiness

| Item | Status | Evidence link | Counsel needed? |
|------|--------|---------------|-----------------|
| Brand-name trademark search | Complete | Clean search for "Operator Node" in local IPOPHL database. | No |
| Open-source license compliance | Complete | Uses MIT licensed React/Vite/Workbox, and LGPL/Apache ESP32 libraries. | No |
| Copyleft scan | Clean | No GPLv3 code in firmware or client. | No |
| Third-party assets licensed | Clean | Built-in icons are local SVG. | No |
| AI vendor output ownership | Clean | N/A (no AI model used) | No |
| Written IP assignment | Complete | Group 6 joint assignment of prototype IP to LGU partner. | Yes |

---

## 6. Platform Compliance

N/A — served entirely as an offline browser page over a local SoftAP. No deployment to Apple App Store or Google Play Store is planned.

---

## Self-Check

- [x] Section 0 declares every market; demo-vs-commercial reachability is stated honestly
- [x] Section 1 has one row per processing activity, each with a retention period
- [x] Section 2 filled for the Philippines jurisdiction
- [x] Section 3 "Yes" mitigation is stated; disclaimer banner is at the top
- [x] Section 4 ToU clauses presence-checked (drafting left to counsel)
- [x] Section 5 flags trademark / open-source compliance
- [x] Section 6 marked N/A
- [x] This document maps obligations and escalates — it does not give legal advice

---

*Document suite complete. Sequence: [BRD](../docs/BRD_Template.md) → [PRD](../docs/PRD_Template.md) → [DSD](../docs/DSD_Template.md) → [SDD](../docs/SDD_Template.md) → [QAD](../docs/QAD_Template.md) → [CLR](../docs/CLR_Template.md) → [GTM_Template.md](../docs/GTM_Template.md).*
