# Smart Classroom ESP32 Simulation Guide (Wokwi)

**Team 06 — CSE 315 / 316 (University of Asia Pacific)**

This simulation is **100% synchronized with [`working.ino`](file:///d:/Study/3.2/CSE-315/Project/simulation/working.ino)** logic and adapted for the **ESP32** microcontroller.

---

## 📁 Project Files

| File | Description |
| :--- | :--- |
| **[`sketch.ino`](file:///d:/Study/3.2/CSE-315/Project/simulation/wokwi_esp32_simulation/sketch.ino)** | Full ESP32 firmware with 100% `working.ino` logic + **Firebase Realtime Database Sync** via Wi-Fi. |
| **[`diagram.json`](file:///d:/Study/3.2/CSE-315/Project/simulation/wokwi_esp32_simulation/diagram.json)** | Pre-wired visual circuit with 5 RFID Card Buttons, 2 IR Direction Buttons, MFRC522 RFID, DHT22, 16x2 I2C LCD, Buzzer, 4 Relays, and the **5V DC Cooling Fan**. |
| **[`fan.chip.json`](file:///d:/Study/3.2/CSE-315/Project/simulation/wokwi_esp32_simulation/fan.chip.json)** | Wokwi Custom Chip definition for the visual 2-pin DC Cooling Fan. |
| **[`fan.chip.c`](file:///d:/Study/3.2/CSE-315/Project/simulation/wokwi_esp32_simulation/fan.chip.c)** | C implementation of the 25 FPS rotating 4-blade propeller animation. |
| **[`PHASE_2_FIREBASE_GUIDE.md`](file:///d:/Study/3.2/CSE-315/Project/simulation/wokwi_esp32_simulation/PHASE_2_FIREBASE_GUIDE.md)** | Step-by-step 3-minute Firebase Realtime Database setup and verification guide. |
| **[`firebase_template.json`](file:///d:/Study/3.2/CSE-315/Project/simulation/wokwi_esp32_simulation/firebase_template.json)** | Sample JSON schema to import into Firebase Console for quick testing. |
| **[`libraries.txt`](file:///d:/Study/3.2/CSE-315/Project/simulation/wokwi_esp32_simulation/libraries.txt)** | Auto-installer library list for Wokwi. |

---

## 🌀 How to Enable the Animated DC Cooling Fan in Wokwi

Wokwi has no built-in DC motor component. We created a **custom 2-pin DC Cooling Fan chip** (`chip-fan`) that renders an animated, rotating 4-blade propeller whenever Relay 1 is activated!

To enable it in your Wokwi browser project:
1. In the Wokwi code editor, look above the code tabs and click the little drop-down arrow **`▾`** (or the **`+`** / **"New File"** button).
2. Name the first file: `fan.chip.json` and paste the contents of [`fan.chip.json`](file:///d:/Study/3.2/CSE-315/Project/simulation/wokwi_esp32_simulation/fan.chip.json).
3. Click **"New File"** again, name it: `fan.chip.c` and paste the contents of [`fan.chip.c`](file:///d:/Study/3.2/CSE-315/Project/simulation/wokwi_esp32_simulation/fan.chip.c).
4. Save and run the simulation! Whenever temperature is $\ge 27.0^\circ\text{C}$ and someone is in class, the cooling fan will spin visibly in real time!

---

## 🔌 Physical Version: Safe Fan Hardware Wiring Guide

In your physical classroom prototype, you will connect an actual 5V or 12V DC cooling fan to **Relay Channel 1** as follows:

```
[External 5V / 12V DC Supply (+)] ───► Relay 1 COM
[Relay 1 NO]                      ───► Fan (+) Positive (Red Wire)
[External Power Supply GND / (-)] ───► Fan (-) Negative (Black Wire) & ESP32 GND
[ESP32 GPIO 26]                   ───► Relay 1 IN1
[ESP32 5V (VIN)]                  ───► Relay VCC
[ESP32 GND]                       ───► Relay GND
```

> [!CAUTION]
> **CRITICAL HARDWARE PROTECTION — 1N4007 Flyback Diode (Physical Build Only)**:
> In the physical prototype, turning off the DC motor will produce an inductive back-EMF voltage spike (often 50V–100V+).
> Without protection, this spike will pit relay contacts, disrupt power rails, and cause erratic resets or damage to the ESP32.
>
> **Wiring the Diode across Fan Terminals:**
> ```
>                   ┌──────────────────────┐
>                   │ 1N4007 Flyback Diode │
>                   │                      │
>                   │   [Cathode (Stripe)] │
>                   │          │           │
>                   │          ▼           │
> Relay 1 [NO] ─────┴──────────┬───────────┴─────► Fan (+) [Red Wire]
>                              │
>                             ▲│ (Reverse-biased: conducts only when power cuts)
>                              │
> ESP32 GND / DC (-) ──────────┴─────────────────► Fan (-) [Black Wire]
>                       [Anode (Black end)]
> ```
> * **Silver Stripe (Cathode)**: MUST connect to **Fan (+) / Relay NO**.
> * **Black Body (Anode)**: Connects to **Fan (-) / Ground**.
> * *(Warning: If connected backwards, it will short-circuit your power supply as soon as Relay 1 turns ON!)*
>
> *(Note: The Wokwi simulator runs pure digital logic and does not emulate inductive spike physics, which is why it is omitted from the simulation schematic).*

---

## 🎯 Automation Rules (From `working.ino`)

1. **Room Occupancy**:
   $$\text{occupied} = \text{teacherPresent} \lor (\text{studentCount} > 0)$$
2. **Room Light (Relay 2)**:
   * **ON** if $\text{occupied}$ (Teacher is inside **OR** Students are inside).
   * **OFF** only when classroom is completely empty.
3. **Projector (Relay 4)**:
   * **ON** only when $\text{teacherPresent}$ is **true**.
   * **OFF** whenever teacher leaves (even if students remain).
4. **Ceiling Fan (Relay 1)**:
   * Controlled by occupancy & temperature:
   * Turns **ON** when $\text{occupied}$ and $\text{Temp} \ge 27.0^\circ\text{C}$.
   * Turns **OFF** when $\text{Temp} \le 25.5^\circ\text{C}$ or classroom becomes empty.
5. **Air Conditioner / AC (Relay 3)**:
   * Turns **ON** when $\text{occupied}$ and $\text{Temp} \ge 30.0^\circ\text{C}$.
   * Turns **OFF** when $\text{Temp} \le 28.5^\circ\text{C}$ or classroom becomes empty.

---

## 🧪 Step-by-Step Test: Teacher Leaves While Student Stays Inside

To verify that the light and AC do **not** turn off when the teacher leaves:

1. **Student Enters**:
   * Click **S1 (Blue button)** $\rightarrow$ LCD shows: `Student card OK / Hasanul`.
   * Click **IR1 (Green button)** then **IR2 (Red button)** $\rightarrow$ Hasanul enters!
   * **Result**:
     * `studentCount = 1`, `teacherPresent = false`.
     * **Light Relay & Yellow LED turn ON**!
     * Projector stays **OFF**.

2. **Teacher Enters**:
   * Click **T: Sayma Ma'am (Orange button)** $\rightarrow$ LCD shows: `Teacher card OK / Sayma Ma'am`.
   * Click **IR1** then **IR2** $\rightarrow$ Sayma Ma'am enters!
   * **Result**:
     * `studentCount = 1`, `teacherPresent = true`.
     * **Projector Relay & White LED turn ON**!
     * **Light stays ON**!

3. **Teacher Leaves (The Critical Test)**:
   * Click **T: Sayma Ma'am (Orange button)** $\rightarrow$ `Sayma Ma'am tapped card`.
   * Click **IR2 (Red button)** then **IR1 (Green button)** $\rightarrow$ Sayma Ma'am exits!
   * **Result**:
     * `teacherPresent = false`, but `studentCount` is **STILL 1**!
     * **Projector turns OFF** (because teacher left).
     * **Light STAYS ON** (because Hasanul is still inside!).
     * **Fan / AC STAY ON** if temperature is warm (because classroom is still occupied!).
     * LCD Page 0 shows: `Tchr: Absent`, `Students: 1`.

4. **Student Leaves**:
   * Click **S1 (Blue button)** $\rightarrow$ `Hasanul tapped card`.
   * Click **IR2** then **IR1** $\rightarrow$ Hasanul exits!
   * **Result**:
     * `studentCount = 0`, `teacherPresent = false`.
     * Classroom is empty $\rightarrow$ **Light turns OFF**, Fan turns **OFF**, AC turns **OFF**!

---

## ⌨️ Serial Monitor Shortcuts

You can also run all tests by typing these into the Serial Monitor at the bottom:
* `T`   $\rightarrow$ Tap Teacher Card (Sayma Ma'am)
* `S1`  $\rightarrow$ Tap Student 1 Card (Hasanul)
* `S2`  $\rightarrow$ Tap Student 2 Card (Jubair)
* `S3`  $\rightarrow$ Tap Student 3 Card (Maria)
* `U`   $\rightarrow$ Tap Unknown Card
* `IN`  $\rightarrow$ Simulate Door Entry crossing (IR1 then IR2)
* `OUT` $\rightarrow$ Simulate Door Exit crossing (IR2 then IR1)
* `STATUS` $\rightarrow$ Print complete formatted table of all people and appliances
