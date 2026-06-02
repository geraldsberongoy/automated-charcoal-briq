# Nextion HMI Touchscreen Design Guide (`Nextion_HMI_Design.md`)

This document serves as the UI blueprint for designing the on-machine operator console inside the **Nextion Editor** software (targeting a Nextion 4.3" Intelligent/Basic screen, resolution `480x272`).

---

## 1. Global Touchscreen Configuration
*   **Resolution:** 480 × 272 pixels (Landscape)
*   **Baud Rate:** 9600 bps (UART Serial2: ESP32 GPIO 16 RX / GPIO 17 TX)
*   **Theme Palette:**
    *   Background: Dark Charcoal (`#0A0C10`)
    *   Active/Glow: Neon Teal (`#00D4AA`)
    *   Alert/Breach: Safety Red (`#EF4444`)
    *   Text: High-Contrast White (`#FFFFFF`)

---

## 2. HMI Screen / Page Hierarchy

Below are the 6 screens to create in the Nextion project:

### Page 0: `soak` (Paper Soaking workspace)
*   **Purpose:** Monitors liquid levels during initial paper soaking.
*   **Visual Layout Mockup:**
    ```
    +-------------------------------------------------------------+
    | PREPARATION PHASE (SOAKING)                     [AP ACTIVE] |
    |                                                             |
    |  SOLENOID VALVE: [ OPEN ]                                   |
    |  GRINDER MOTOR: [ OFF ]                                     |
    |                                                             |
    |  WATER FLOW INTAKE:  2.45 L / 3.00 L                        |
    |  [=========PROGRESS BAR=========               ] 81%        |
    +-------------------------------------------------------------+
    ```
*   **Component Inventory:**
    *   `t0` (Text): "Water Flow Intake: X.XX L / 3.00 L"
    *   `j0` (Progress Bar): Volumetric flow progress bar.
    *   `b0` (Button): "SOAK DONE" (Unlocked when flow >= 3.0L, sends trigger `state=2`)

---

### Page 1: `grind` (Paper Slurry Grinding workspace)
*   **Purpose:** Allows grinder activation and monitors current draw.
*   **Visual Layout Mockup:**
    ```
    +-------------------------------------------------------------+
    | GRINDING PHASE                                  [AP ACTIVE] |
    |                                                             |
    |  GRINDER: [ RUNNING ]                    [ START PULVERIZE ]|
    |  CURRENT DRAW: 4.2 A                     [ STOP PULVERIZE  ]|
    |                                                             |
    |  [========================PROGRESS BAR=====================] |
    +-------------------------------------------------------------+
    ```
*   **Component Inventory:**
    *   `b0` (Button): "Start Pulverize" (Sends Serial trigger `grind=1`)
    *   `b1` (Button): "Stop Pulverize" (Sends Serial trigger `grind=0`)
    *   `t0` (Text): "Current Draw: X.X A"
    *   `j0` (Progress Bar): Grinder current level progress bar.
    *   `b2` (Button): "GRIND DONE" (Sends Serial trigger `state=3`)

---

### Page 2: `mix` (Blending & Homogenization)
*   **Purpose:** Monitors mixing duration and countdown.
*   **Visual Layout Mockup:**
    ```
    +-------------------------------------------------------------+
    | COMBINING PHASE (MIXING)                        [AP ACTIVE] |
    |                                                             |
    |  PLANETARY MIXER MOTOR: [ ACTIVE ]                          |
    |  SPEED: 60 RPM                                              |
    |                                                             |
    |  TIMER REMAINING: 00:18                                     |
    |  [=======================>                     ] 40%        |
    +-------------------------------------------------------------+
    ```
*   **Component Inventory:**
    *   `t0` (Text): "Planetary Mixer Motor: ACTIVE"
    *   `t1` (Text): "Timer Remaining: MM:SS"
    *   `j0` (Progress Bar): Count down percentage fill.
    *   `b0` (Button): "MIX DONE" (Sends Serial trigger `state=4`)

---

### Page 3: `mold` (Compression & Compaction Workspace)
*   **Purpose:** Controls and monitors compaction pressure.
*   **Visual Layout Mockup:**
    ```
    +-------------------------------------------------------------+
    | COMPRESSION PHASE (COMPACTION)              [LIMITS: SAFE]  |
    |                                                             |
    |  COMPACTION PRESSURE: 1450 N / 2000 N                       |
    |  [=========================>                   ] 72%        |
    |                                                             |
    |   [ MANUAL EXTEND ]    [ MANUAL RETRACT ]   [CONFIRM MOLD]  |
    +-------------------------------------------------------------+
    ```
*   **Component Inventory:**
    *   `t0` (Text): "Compaction Pressure: XXXX N / 2000 N"
    *   `j0` (Progress Bar): Pressure percentage.
    *   `b0` (Button): "Manual Extend" (Sends trigger `actuator=down`)
    *   `b1` (Button): "Manual Retract" (Sends trigger `actuator=up`)
    *   `b2` (Button): "CONFIRM MOLD" (Unlocked when load >= 2000N, sends trigger `state=5`)

---

### Page 4: `dry` (Thermal Curing/Drying oven workspace)
*   **Purpose:** Monitors halogen oven temperature and relative humidity.
*   **Visual Layout Mockup:**
    ```
    +-------------------------------------------------------------+
    | THERMAL CURING (DRYING)                      [LOGGING: OK]  |
    |                                                             |
    |  ENCLOSURE TEMP : 42.5 C                                    |
    |  EXHAUST HUMIDITY: 24.1 %  (Target: <= 15.0%)               |
    |  [=======================>                     ] 24%        |
    |                                                             |
    |                                            [ UNLOAD CHUNK ] |
    +-------------------------------------------------------------+
    ```
*   **Component Inventory:**
    *   `t0` (Text): "Enclosure Temp: XX.X C"
    *   `t1` (Text): "Exhaust Humidity: XX.X %"
    *   `j0` (Progress Bar): Humidity level progress bar (inverse fill).
    *   `b0` (Button): "UNLOAD CHUNK" (Unlocked when RH <= 15%, sends trigger `state=1`)

---

### Page 5: `fault` (Critical Safety Halt Overlay)
*   **Purpose:** Blocks UI inputs during interlock open / casing breach.
*   **Visual Layout Mockup:**
    ```
    +-------------------------------------------------------------+
    | !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!   |
    |                   CRITICAL SAFETY HALT                      |
    |                                                             |
    |     CASING BREACH DETECTED - ACTUATOR DISPLACEMENT 0%       |
    |                                                             |
    |     [ CLOSE SAFETY COVER AND TOUCH TO RESET FAULT ]         |
    | !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!   |
    +-------------------------------------------------------------+
    ```
*   **Component Inventory:**
    *   `t0` (Text): Flashing warning alert.
    *   `b0` (Button): Fullscreen touch trigger to clear fault.

---

## 3. Serial Communication Protocol (Nextion <-> ESP32)

Nextion transmits messages using UART packet chunks ending in `0xFF 0xFF 0xFF`.
*   **Operator state confirmations:** Nextion writes `state=X` to Serial.
*   **Manual motor controls:** Grinder trigger writes `grind=1`, retract compactor writes `actuator=up`.
*   **ESP32 telemetry updates:** The ESP32 writes to Nextion object text attributes periodically (e.g. `soak.t0.txt="Water: 2.45L"`).
