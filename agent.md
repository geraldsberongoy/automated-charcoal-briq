# Agent Orchestration Guide (`agent.md`)

This guide outlines how AI agents should orchestrate, read, and write files in the [docs](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs) folder for the **AUTOBRIQ: Fully Automated Paper-Charcoal Briquette Machine** project.

**Source of truth:** the thesis, text copy in [chapters/AUTOBRIQ_fully_automated_thesis.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/chapters/AUTOBRIQ_fully_automated_thesis.md). The earlier semi-automated documents in `chapters/` are superseded. Read [docs/PROJECT_CONTEXT.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/PROJECT_CONTEXT.md) first: it summarises the thesis, lists what is built, and tracks every place where the code or the docs differ from the thesis.

---

## 1. Documentation Lifecycle & Dependency Graph

When updating features, modifying firmware logic, or editing web dashboard views, documentation must be processed sequentially. Do not bypass stages.

```
┌──────────────────┐      ┌──────────────────┐      ┌──────────────────┐
│  BRD (Business)  │ ───> │  PRD (Product)   │ ───> │   DSD (Design)   │
└──────────────────┘      └──────────────────┘      └──────────────────┘
                                                              │
                                                              ▼
┌──────────────────┐      ┌──────────────────┐      ┌──────────────────┐
│ GTM (Go-To-Market│ <─── │ CLR (Legal/Comp) │ <─── │  SDD (Software)  │
└──────────────────┘      └──────────────────┘      └──────────────────┘
        ▲                                                     │
        │                                                     ▼
        └─────────────────────────────────────────────── QAD (Testing)
```

### Document Registry

| Sequence | Document | Template File | Output/Refilled Target |
|:---:|---|---|---|
| 0 | **Project Context** | [PROJECT_CONTEXT.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/PROJECT_CONTEXT.md) | Thesis summary, build status, known gaps |
| 1 | **Business Requirements** | [BRD_Template.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/BRD_Template.md) | In-place refilled template |
| 2 | **Product Requirements**  | [PRD_Template.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/PRD_Template.md) | In-place refilled template |
| 3 | **Design Specification**   | [DSD_Template.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/DSD_Template.md) | In-place refilled template |
| 4 | **System / Software Design**| [SDD_Template.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/SDD_Template.md) | In-place refilled template |
| 5 | **Quality Assurance / Test**| [QAD_Template.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/QAD_Template.md) | In-place refilled template |
| 6 | **Compliance & Legal**    | [CLR_Template.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/CLR_Template.md) | In-place refilled template |
| 7 | **Go-To-Market Strategy** | [GTM_Template.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/GTM_Template.md) | In-place refilled template |
| - | **Nextion HMI Design**    | [Nextion_HMI_Design.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/Nextion_HMI_Design.md) | On-machine touchscreen blueprint |

---

## 2. Project Variables & Parameters

Ensure all documents maintain consistent parameters. Any deviation in these values triggers validation failures:

*   **Project Name:** `AUTOBRIQ: Fully Automated Paper-Charcoal Briquette Machine`
*   **Project Slug:** `automated-charcoal-briq`
*   **Owner:** `PUP Computer Engineering Group 6 (Añonuevo, Berongoy, Galdiano, Tiedra)`
*   **Primary Target Market / Jurisdiction:** `Cavinti, Laguna, Philippines (Barangay Anglas and Barangay Poblacion; RA 10173 DPA Compliance)`
*   **Core Hardware Target:** `ESP32-WROOM-32D Dual-Core MCU (DOIT ESP32 DEVKIT V1)`
*   **On-machine HMI:** `Nextion NX8048T070 7.0" (800x480), UART`
*   **Machine Cycle:** `Water dispensing, soaking and mixing in a shared container, gravity discharge to a partitioned mold, compression molding, press retraction. No thermal drying; briquettes are air-dried outside the machine.`
*   **Batch Target:** `At least 20 briquettes per batch (partitioned pull-out mold, one fixed rectangular-prism size)`
*   **Compression Force Threshold:** `Preset by calibration; actuator thrust is 1,500 N; load cell is 300 kg S-type + HX711`
*   **Water Level:** `Selected 30% to 100% of container capacity (preset 100%); dispensed by YF-S201 pulse count`
*   **Mix Design:** `Paper : charcoal about 1 : 1 by weight; wet mix 16.13% to 20% paper, 16.13% to 20% charcoal, 67.7% to 60% water`
*   **Power:** `110 W solar panel + EcoFlow RIVER 2 (256 Wh LiFePO4, MPPT); 20 V mixer supply (UME S2415 adjusted) through a 30 A automotive relay`
*   **Relay Module:** `8-channel: CH1 to CH4 actuator directions, CH5 water valve, CH6 automotive relay (mixer), CH7 and CH8 spare`

---

## 3. Orchestration Rules for Agents

### Rule 1: Read-Before-Write
Before proposing changes to the ESP32 C++ firmware code ([NetworkManager.cpp](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/operator-node/src/NetworkManager.cpp)) or React PWA files ([vite.config.js](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/operator-node/dashboard/vite.config.js)), the agent must read [SDD_Template.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/SDD_Template.md) to ensure consistency with data models, endpoints, and caching rules.

### Rule 2: Keep Design Tokens Synchronized
Color values (`--clr-accent`, `--clr-bg-0`, etc.) are defined in [DSD_Template.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/DSD_Template.md) and implemented in [index.css](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/operator-node/dashboard/src/index.css). When editing layout files, verify contrast pairings using the ONDS specification.

### Rule 3: Maintain Offline Posture
Because this deployment is designed for off-grid communities, never introduce code that requires external CDN calls (including fonts, icons and CSS frameworks), public analytics hooks (e.g. Google Analytics), or online API integrations. The PWA must be fully self-contained in the ESP32's LittleFS partition.

### Rule 4: The Thesis Wins
If code, docs and the thesis disagree, the thesis is right. Fix the code or doc, or record the gap in [PROJECT_CONTEXT.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/PROJECT_CONTEXT.md) section 3. Never reintroduce grinding, thermal curing, humidity or temperature sensing, or operator stage-authorization prompts: the thesis removed them.

### Rule 5: Do Not Invent Numbers
Targets the thesis leaves to calibration or measurement (force threshold, mixing time, battery endurance, production rate) stay marked as such until measured.

---

## 4. Verification & Self-Check Checklist

When a documentation change is made, execute these checks:
1.  **Cross-Linking:** Ensure relative paths to sibling documents are correct (e.g. `[PRD](PRD_Template.md)`).
2.  **State Completeness:** The PRD screen inventory must define `empty`, `loading`, `error`, and `success` states for all UI views.
3.  **Halt Handlers:** Any new hardware integration in the SDD must specify a fail-safe shutdown path.
4.  **No Placeholders:** Ensure no `{{PLACEHOLDER}}` tags remain after refilling.
5.  **Thesis Parameters:** Values in section 2 match the thesis and every sibling document.
