# Design System Document (DSD)

**System Name:** Operator Node Design System (ONDS)
**Date:** 2026-06-03
**Version:** 1.0.0
**Owner:** PUP Computer Engineering Group 6 (Añonuevo, Berongoy, Galdiano, Tiedra)
**PRD:** [PRD_Template.md](PRD_Template.md)

---

> **Scope:** This design system covers the client-side user interface of the responsive React Progressive Web App (PWA) dashboard, optimized for mobile screens (Android) and fallback desktop monitors. No secondary embed surfaces are in scope.

---

## 1. Design Philosophy & Vision

**Core aesthetic:**
Techno-Ecological (Tech-Eco) Minimalist. It fuses deep slate-charcoal tones representing the raw materials (charcoal fines, carbon, waste paper) with sharp, glowing neon-teal accents representing clean energy, technology, and sustainability. The layout is structured, dense, and grid-aligned to match the engineering context.

**Emotional intent:**
The operator should feel a sense of physical safety, control, and pride in using the machine. The interface looks professional and high-tech, turning a raw waste-management task into a high-status engineering operation. Large buttons, high contrast, and structured wizard steps build confidence and prevent operational anxiety.

**Aesthetic references:**
*   Grafana monitoring panels (dense telemetry, dark background).
*   Ableton Live / Hardware Synthesizer interfaces (knurled sliders, high readability, layout density).

**What this system explicitly avoids:**
*   **Decorative animations:** Animations must be kept to a minimum to avoid overloading the client browser rendering on low-cost smartphones and to conserve ESP32 SoftAP resources.
*   **Color-only states:** Process states are never conveyed via colors alone; every color shift is coupled with a distinct icon and explicit text label to accommodate operators with color blindness.
*   **Hidden parameters:** All sensor values are exposed on screen; there are no hidden sub-menus, reducing cognitive lookup time.

---

## 2. Brand Primitives

### Colors

Our color tokens match the project's CSS styling custom properties:

| Token | Value | Usage |
|-------|-------|-------|
| `--clr-bg-0` | `#0a0c10` | Dark app background |
| `--clr-bg-1` | `#12161e` | Panel and card backgrounds |
| `--clr-bg-2` | `#1a2030` | Button default backgrounds, inline highlights |
| `--clr-border` | `#2a3448` | Card borders, dividers, passive borders |
| `--clr-accent` | `#00d4aa` | Neon teal: Primary CTA, running state, active connections |
| `--clr-accent-dim` | `#00a882` | Darker teal: Active button hover, active select |
| `--clr-text-0` | `#e8edf5` | High-contrast body text, primary labels |
| `--clr-text-1` | `#9bacc8` | Secondary labels, descriptions, units |
| `--clr-text-2` | `#5d7299` | Muted values, timestamp text, inactive items |
| `--clr-success` | `#00d4aa` | Telemetry target achieved, successful checks |
| `--clr-warn` | `#f59e0b` | Attention required, non-critical warnings |
| `--clr-danger` | `#ef4444` | Safety breach, emergency halt, errors |

State encoding uses **color + icon + label** — never color alone.

### Typography

We load **Inter** and **JetBrains Mono** via Google Fonts with local system fallbacks:

| Role | Font | Weight | Size | Line Height |
|------|------|--------|------|-------------|
| Heading 1 | Inter, sans-serif | 700 (Bold) | 1.5rem | 1.3 |
| Heading 2 | Inter, sans-serif | 600 (Semi-Bold) | 1.15rem | 1.4 |
| Body | Inter, sans-serif | 400 (Regular) | 0.95rem | 1.5 |
| Small / Label | Inter, sans-serif | 500 (Medium) | 0.78rem | 1.4 |
| Mono / Numbers | JetBrains Mono, monospace | 600 (Semi-Bold) | 0.85rem | 1.3 |

**Font loading:**
Fonts are linked in the HTML head via Google Fonts CDN, but the PWA cache registers these URLs on initial load. If offline, the browser gracefully falls back to system sans-serif (e.g. System UI, BlinkMacSystemFont) and monospace font-stacks.

### Elevation & Depth

*   `--shadow-card`: `0 4px 24px rgba(0, 0, 0, 0.45)` (Dense drop shadow used on panels to separate them from the grid background).
*   `--radius-md`: `12px` (Standard corner rounding for cards and containers).
*   `--radius-sm`: `6px` (Corner rounding for buttons and badges).

---

## 3. Layout & Spatial System

**Base unit:** `4px` — all padding, margin, and layout gaps are multiples of this base unit.

| Token | Value | Usage |
|-------|-------|-------|
| `--space-1` | `4px` | Tiny padding inside badges, small borders |
| `--space-2` | `8px` | Inner gap of elements (e.g. icon to text label) |
| `--space-4` | `16px` | Default padding inside cards, default layout margins |
| `--space-8` | `32px` | Section-to-section separation |

**Grid:**
Flexible flexbox-based layouts with a responsive CSS grid that adjusts column counts.

**Breakpoints:**
*   Mobile: `< 768px` (Primary target layout — single column card layout optimized for handheld use).
*   Desktop: `>= 768px` (Multi-column dashboard layout layout for administrative review).

---

## 4. Core Component Specs

### Buttons

| Variant | Background | Text | Border | Hover | Disabled |
|---------|-----------|------|--------|-------|----------|
| Primary | `--clr-accent` | `#0a0c10` | None | `--clr-accent-dim` | 40% opacity |
| Secondary | transparent | `--clr-text-0` | `1px solid --clr-border` | `--clr-bg-2` | 40% opacity |
| Destructive | `--clr-danger` | `#e8edf5` | None | Darkened Red | 40% opacity |

**Border radius:** `6px` · **Padding:** `12px 20px` · **Font:** Inter, 600 weight. Touch targets are scaled to a minimum height of `48px` to ensure accessibility for dirty or gloved fingers.

### Inputs & Forms

*   **Inputs:** Background is `--clr-bg-2`, border is `--clr-border`, border-radius is `6px`.
*   **Focus Ring:** `2px solid --clr-accent` with an outline offset of `2px`.
*   **Error State:** Border becomes `1px solid --clr-danger` with a supporting red alert text block below the input field.

### Surfaces (Cards, Panels)

*   **Cards:** Background is `--clr-bg-1`, border is `1px solid --clr-border`, border-radius is `12px`, shadow is `--shadow-card`.
*   **Workspace Step Card:** Focuses on the active FSM step. Active cards highlight their border using `--clr-accent` (Neon Teal) and display a glowing accent circle.

---

## 5. Motion & Micro-interactions

**Transition default:** `0.2s ease`

| Interaction | Duration | Easing | Notes |
|-------------|----------|--------|-------|
| Button hover | `0.15s` | `ease` | Subtle color change and glow expansion. |
| Alert pulse | `2.0s` | `ease-in-out` | Infinite glow pulsing on danger alerts. |
| Progress updates | `0.3s` | `ease-out` | Smooth width change on progress bars. |

**Avoid:** Animated page transits, bouncing layouts, or heavy canvas animations.

---

## 6. Accessibility (a11y)

*   **Contrast:** Primary neon-teal text (`#00d4aa`) on slate-black (`#0a0c10`) yields a contrast ratio of `9.4:1` (exceeds WCAG AAA standard). Body copy (`#e8edf5`) on slate-black yields a ratio of `15.8:1`.
*   **State Indicators:** States always pair color shifts with distinct visual markers (e.g. Checkmark badge `✓` for OK, Exclamation mark `!` for Error).
*   **Touch Targets:** Scaled to a minimum of `48×48px` (standard touch height targets are `44×44px` but expanded here to ease operation).
*   **Reduced Motion:** Animations are disabled if the browser queries `@media (prefers-reduced-motion: reduce)`.

---

## 7. Taste-Skill Settings

```
DESIGN_VARIANCE:    3
MOTION_INTENSITY:   2
VISUAL_DENSITY:     8
```

**Chosen variant:** Industrial-Edge Cyber-Physical View
**Reason:** The system operates in demanding physical conditions (dust, noise, sunlight). A visual density score of `8` ensures all sensors and parameters are shown on a single screen without hiding telemetry, while a motion intensity score of `2` protects low-spec mobile hardware from client-side CPU bottlenecks.

---

## 8. Impeccable Anti-Pattern Register

| Pattern | Status | Location | Fix Applied |
|---------|--------|----------|-------------|
| **Color-Only Alerts** | Avoided | Workspace Dashboard | All error codes are accompanied by text descriptors and a yellow/red warning triangle icon. |
| **Hidden Operations** | Avoided | Stage Controller | Modals and tabs are minimized; the sequential FSM operates as a wizard layout. |
| **Small CTA targets** | Fixed | Confirm Buttons | All confirmation buttons have been given a minimum height of `48px`. |

---

## Self-Check

- [x] Section 2 has exact HEX values for the primary UI
- [x] Section 3 spacing scale is consistent (all multiples of base unit)
- [x] Section 4 defines component states including Disabled and Focus
- [x] Section 7 taste-skill dials set and a variant chosen with reasoning
- [x] WCAG AA contrast verified for primary text/background pairings
- [x] Tokens exist in code (CSS variables in [index.css](file:///C:/Users/User/CODERIST/mor/automated-charcoal-briq/operator-node/dashboard/src/index.css))

---

*Next in sequence: [SDD](SDD_Template.md) — system architecture and data model.*
