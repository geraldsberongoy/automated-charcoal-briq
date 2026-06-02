# Agent Orchestration Guide (`agent.md`)

This guide outlines how AI agents should orchestrate, read, and write files in the [docs](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs) folder for the **Semi-Automated Paper-Charcoal Briquette Machine** project.

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
| 1 | **Business Requirements** | [BRD_Template.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/BRD_Template.md) | In-place refilled template |
| 2 | **Product Requirements**  | [PRD_Template.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/PRD_Template.md) | In-place refilled template |
| 3 | **Design Specification**   | [DSD_Template.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/DSD_Template.md) | In-place refilled template |
| 4 | **System / Software Design**| [SDD_Template.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/SDD_Template.md) | In-place refilled template |
| 5 | **Quality Assurance / Test**| [QAD_Template.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/QAD_Template.md) | In-place refilled template |
| 6 | **Compliance & Legal**    | [CLR_Template.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/CLR_Template.md) | In-place refilled template |
| 7 | **Go-To-Market Strategy** | [GTM_Template.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/GTM_Template.md) | In-place refilled template |

---

## 2. Project Variables & Parameters

Ensure all documents maintain consistent parameters. Any deviation in these values triggers validation failures:

*   **Project Name:** `Semi-Automated Paper-Charcoal Briquette Machine`
*   **Project Slug:** `automated-charcoal-briq`
*   **Owner:** `PUP Computer Engineering Group 6 (Añonuevo, Berongoy, Galdiano, Tiedra)`
*   **Primary Target Market / Jurisdiction:** `Cavinti, Laguna, Philippines (Philippines RA 10173 DPA Compliance)`
*   **Core Hardware Target:** `ESP32-WROOM-32D Dual-Core MCU (DOIT ESP32 DEVKIT V1)`
*   **Compaction Force Threshold:** `2000N`
*   **Drying Cutoff Threshold:** `<= 15% Relative Humidity`
*   **Wetting Ratio:** `1:1 dry-weight water-to-paper slurry`

---

## 3. Orchestration Rules for Agents

### Rule 1: Read-Before-Write
Before proposing changes to the ESP32 C++ firmware code ([NetworkManager.cpp](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/operator-node/src/NetworkManager.cpp)) or React PWA files ([vite.config.js](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/operator-node/dashboard/vite.config.js)), the agent must read [SDD_Template.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/SDD_Template.md) to ensure consistency with data models, endpoints, and caching rules.

### Rule 2: Keep Design Tokens Synchronized
Color values (`--clr-accent`, `--clr-bg-0`, etc.) are defined in [DSD_Template.md](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/docs/DSD_Template.md) and implemented in [index.css](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/operator-node/dashboard/src/index.css). When editing layout files, verify contrast pairings using the ONDS specification.

### Rule 3: Maintain Offline Posture
Because this deployment is designed for off-grid communities, never introduce code that requires external CDN calls, public analytics hooks (e.g. Google Analytics), or online API integrations. The PWA and firmware must be entirely self-contained.

---

## 4. Verification & Self-Check Checklist

When a documentation change is made, execute these checks:
1.  **Cross-Linking:** Ensure relative paths to sibling documents are correct (e.g. `[PRD](PRD_Template.md)`).
2.  **State Completeness:** The PRD screen inventory must define `empty`, `loading`, `error`, and `success` states for all UI views.
3.  **Halt Handlers:** Any new hardware integration in the SDD must specify a fail-safe shutdown path.
4.  **No Placeholders:** Ensure no `{{PLACEHOLDER}}` tags remain after refilling.
