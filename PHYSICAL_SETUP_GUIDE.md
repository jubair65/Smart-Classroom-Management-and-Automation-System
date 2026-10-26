# 🏫 Smart Classroom Management System — Physical Setup Guide

**Team 06 — CSE 315 / 316 (University of Asia Pacific)**  
*ESP32 + Firebase Realtime Database + Android Mobile App — Complete Hardware Assembly & Troubleshooting Manual*

---

## 📐 Physical Hardware Overview

```
                      ┌───────────────────────────────────────────┐
                      │          ESP32 DevKit V1 (30-Pin)         │
                      │          [Dual-Core Xtensa @ 240MHz]      │
                      └─────────────────────┬─────────────────────┘
                                            │
         ┌──────────────────────────────────┼──────────────────────────────────┐
         │                                  │                                  │
  [ CORE 1: HARDWARE LOOP ]                 │                   [ CORE 0: ASYNC CLOUD ]
  (0ms latency, 100% responsive)            │                   (Non-blocking FreeRTOS Task)
         │                                  │                                  │
         ├─► RFID MFRC522 (VSPI)            │                                  ├─► WiFi 2.4GHz
         ├─► Dual IR Gate (IR1 / IR2)       │                                  ├─► Firebase Realtime DB
         ├─► DHT22 Temp / Humidity          │                                  │   • /classroom/live
         ├─► I2C 16x2 LCD Display           │                                  │   • /classroom/alerts
         ├─► 5V Active Buzzer               │                                  │   • /classroom/history
         └─► 4-Channel Relay Module         │                                  └─► Android Mobile App
             • CH1: 5V DC Fan               │
             • CH2: Light (Yellow LED)      │
             • CH3: AC (Blue LED)           │
             • CH4: Projector (White LED)   │
```

---

## 🧩 Components & Bill of Materials (BOM)

| # | Component | Specification | Qty | Role in Prototype |
|---|-----------|--------------|-----|-------------------|
| 1 | **ESP32 DevKit V1** | 30-pin, Dual-Core 240MHz, Wi-Fi + BT | 1 | Master Controller (runs Core 0 background sync + Core 1 sensor loop) |
| 2 | **ESP32 30-Pin Expansion Board** | Breakout with G-V-S headers & DC Jack | 1 | Distributes 5V, 3.3V, and GND rails cleanly |
| 3 | **MFRC522 RFID Module** | 13.56 MHz SPI Reader (**3.3V ONLY**) | 1 | Scans student & teacher attendance cards |
| 4 | **MIFARE Classic 1K Cards** | 13.56 MHz ISO 14443A RFID Cards | 4 | Registered to Teacher + 3 Students |
| 5 | **DHT22 (AM2302) Sensor** | Temp & Humidity Module (±0.5°C accuracy) | 1 | Triggers Fan ($\ge$27.0°C) and AC ($\ge$30.0°C) automation |
| 6 | **16×2 LCD + I2C Backpack** | PCF8574 I2C adapter (Address `0x27`) | 1 | Live rotating display for occupancy, temp & appliances |
| 7 | **Active 5V Buzzer** | Self-oscillating buzzer (**not passive**) | 1 | Audio feedback for card taps & security intruder alarm |
| 8 | **4-Channel 5V Relay Module** | Optocoupler isolated, **Active-LOW** | 1 | Switches power to Fan, Light, AC, and Projector |
| 9 | **IR Obstacle Sensors** | TCRT5000 / LM393 with trimpot | 2 | IR1 (Outside) & IR2 (Inside) for directional door tracking |
| 10 | **5V DC Cooling Fan** | 40x40x10mm 2-pin brushless fan | 1 | Real motor load on Relay CH1 |
| 11 | **1N4007 Rectifier Diode** | 1A, 1000V Flyback Diode | 1 | **CRITICAL:** Clamps inductive voltage kickback across fan |
| 12 | **Prototype LEDs** | Yellow (Light), Blue (AC), White (Proj) | 3 | Appliance prototypes for CH2, CH3, and CH4 |
| 13 | **220Ω Resistors** | 1/4W Current-limiting resistors | 3 | Protects each prototype LED |
| 14 | **Mini Breadboard** | 170-point breadboard | 1 | Holds prototype LEDs and resistor circuits |
| 15 | **Power Source** | **5V / 2A Phone Charger** or DC Barrel Jack | 1 | Prevents 5V voltage sag when all relays and fan turn ON |

---

## 🔌 Complete Pin Assignment Table

| ESP32 Pin | Connected Peripheral | Peripheral Pin | Operating Voltage | Function |
| :--- | :--- | :--- | :--- | :--- |
| **3.3V** | MFRC522, DHT22, IR1, IR2 | `VCC` | **3.3V ONLY** | Clean regulated sensor power rail |
| **VIN (5V)** | LCD, Relay VCC, Relay COMs | `VCC` / `COM` | **5.0V** | High-power rail from USB/DC source |
| **GND** | All Peripherals & Loads | `GND` / `(-)` | **0V** | Common system ground rail |
| **GPIO 4** | MFRC522 RFID | `RST` | 3.3V Logic | RFID Hardware Reset |
| **GPIO 5** | MFRC522 RFID | `SDA (SS / CS)`| 3.3V Logic | SPI Slave Select |
| **GPIO 13** | 4-Channel Relay (CH3) | `IN3` | 3.3V Logic | Controls **AC (Blue LED)** |
| **GPIO 14** | 4-Channel Relay (CH2) | `IN2` | 3.3V Logic | Controls **Room Light (Yellow LED)** |
| **GPIO 17 (`TX2`)**| 4-Channel Relay (CH4) | `IN4` | 3.3V Logic | Controls **Projector (White LED)** |
| **GPIO 18** | MFRC522 RFID | `SCK` | 3.3V Logic | SPI Clock |
| **GPIO 19** | MFRC522 RFID | `MISO` | 3.3V Logic | SPI Master In / Slave Out |
| **GPIO 21** | I2C LCD Backpack | `SDA` | 5V / 3.3V I2C | I2C Serial Data |
| **GPIO 22** | I2C LCD Backpack | `SCL` | 5V / 3.3V I2C | I2C Serial Clock |
| **GPIO 23** | MFRC522 RFID | `MOSI` | 3.3V Logic | SPI Master Out / Slave In |
| **GPIO 25** | 5V Active Buzzer | `(+)` Longer Leg| 3.3V Drive | Audio feedback (card chime & security alarm) |
| **GPIO 26** | 4-Channel Relay (CH1) | `IN1` | 3.3V Logic | Controls **5V DC Fan** |
| **GPIO 27** | DHT22 Sensor | `Data / Pin 2` | 3.3V Logic | Reads room temperature & humidity |
| **GPIO 32** | IR Sensor 1 (Outside) | `OUT` | 3.3V Logic | Door Entry Trigger (Active-LOW) |
| **GPIO 33** | IR Sensor 2 (Inside) | `OUT` | 3.3V Logic | Door Exit Trigger (Active-LOW) |

*(Note: Simulator virtual button pins GPIO 15, 2, 16, 12, 34 are disabled in physical hardware to eliminate floating pin noise).*

---

## 🛠️ Step-by-Step Hardware Assembly & Wiring

---

### Step 1: ESP32 Power & Ground Rails
1. Seat the 30-pin ESP32 into the expansion board socket with the Wi-Fi antenna pointing outward.
2. Establish a **Common Ground Rail** on the mini breadboard by connecting an M-F jumper wire from **ESP32 GND** to the blue `(-)` rail.
3. Power the system with a **5V / 2A phone wall adapter** or through the expansion board's DC barrel jack (avoid weak 500mA PC USB 2.0 ports to prevent brownouts).

---

### Step 2: MFRC522 RFID Reader (SPI Bus)

> [!CAUTION]
> **CRITICAL:** Connect MFRC522 **VCC to 3.3V ONLY**. Connecting 5V will permanently burn the chip!

```
MFRC522 Pin       ESP32 Pin
───────────────────────────
3.3V (VCC)  ───►  3.3V
GND         ───►  GND
RST         ───►  GPIO 4
SDA (SS)    ───►  GPIO 5
SCK         ───►  GPIO 18
MISO        ───►  GPIO 19
MOSI        ───►  GPIO 23
IRQ         ───►  (Leave unconnected)
```

#### Physical RFID Card Roster:
| Card UID | Role | Member Name | System Action |
| :--- | :--- | :--- | :--- |
| **`B1C7030A`** | 👩‍🏫 Teacher | **Sayma Ma'am** | Marks Teacher Present $\rightarrow$ Enables Projector & Attendance |
| **`532E13ADA10001`** | 👨‍🎓 Student 1 | **Hasanul** | Validates entry tap $\rightarrow$ Unlocks 10s door passage |
| **`534BD4AFA10001`** | 👨‍🎓 Student 2 | **Jubair** | Validates entry tap $\rightarrow$ Unlocks 10s door passage |
| **`53148AABA10001`** | 👩‍🎓 Student 3 | **Maria** | Validates entry tap $\rightarrow$ Unlocks 10s door passage |
| *(Any other UID)* | 🚨 Unknown | **Access Denied** | Triggers 3-beep security alarm & pushes Firebase alert |

---

### Step 3: Active Buzzer (GPIO 25)

1. **Polarity:**
   * **`(+)` (Longer leg or marked `+` on sticker):** Connect to **ESP32 GPIO 25**.
   * **`(-)` (Shorter leg):** Connect to **GND**.
2. **Drive Method:**
   * An **Active Buzzer** has an internal oscillator. Driven via simple `digitalWrite(HIGH)` and `digitalWrite(LOW)`. **Do not use `tone()`** (which interferes with the internal oscillator and causes silence).
   * In code, the pin is initialized `LOW` so the buzzer is **100% silent at idle** with zero hissing or noise.
3. **Volume Tip:** Peel off the small protective paper seal covering the sound hole on top for maximum acoustic loudness.

---

### Step 4: Dual IR Direction Door Gate (IR1 & IR2)

Mount the two IR sensor boards **10–15 cm apart** at the doorway:
* **IR Sensor 1 (Outside Face):**
  * `VCC` $\rightarrow$ **3.3V**, `GND` $\rightarrow$ **GND**, `OUT` $\rightarrow$ **GPIO 32**
* **IR Sensor 2 (Inside Face):**
  * `VCC` $\rightarrow$ **3.3V**, `GND` $\rightarrow$ **GND**, `OUT` $\rightarrow$ **GPIO 33**

```
   OUTSIDE CLASSROOM                              INSIDE CLASSROOM
  ┌──────────────────┐                          ┌──────────────────┐
  │   IR SENSOR 1    │                          │   IR SENSOR 2    │
  │ (Outside / Entry)│                          │  (Inside / Exit) │
  └────────┬─────────┘                          └────────┬─────────┘
           │                                             │
           ▼                                             ▼
   Wave IR1 ──► Then IR2                         Wave IR2 ──► Then IR1
   =====================                         =====================
   👉 ENTERING (Entry)                           👈 LEAVING (Exit)
```

* **Trimpot Tuning:** Use a small screwdriver to turn the blue potentiometer on each IR module until its obstacle LED triggers cleanly when your hand is ~15–20 cm away.
* **Entry Logic:** Tap card outside $\rightarrow$ Step through `IR1 -> IR2` within 10s $\rightarrow$ Welcome chime, `totalCount++`.
* **Security Intruder Alert:** Step through `IR1 -> IR2` **without tapping card** $\rightarrow$ 🚨 3-Beep Alarm, access violation logged!
* **Contactless Free Exit:** Step through `IR2 -> IR1` (no card needed to leave) $\rightarrow$ `totalCount--`. When `totalCount == 0`, classroom enters empty mode and shuts all appliances off.

---

### Step 5: DHT22 Temperature & Humidity Sensor (GPIO 27)

```
DHT22 Pin        ESP32 Pin
──────────────────────────
Pin 1 (VCC) ───► 3.3V
Pin 2 (Data)───► GPIO 27
Pin 3 (NC)  ───► (Not connected)
Pin 4 (GND) ───► GND
```
*(If using a bare DHT22 without PCB module, place a 10kΩ pull-up resistor between Pin 2 Data and 3.3V).*

---

### Step 6: 4-Channel Relay Module & Appliance Wiring

#### A. Relay Input Control Headers:
* `VCC` $\rightarrow$ **ESP32 VIN (5V)**
* `GND` $\rightarrow$ **ESP32 GND**
* `IN1` $\rightarrow$ **GPIO 26** (Controls CH1: Fan)
* `IN2` $\rightarrow$ **GPIO 14** (Controls CH2: Room Light)
* `IN3` $\rightarrow$ **GPIO 13** (Controls CH3: AC)
* `IN4` $\rightarrow$ **GPIO 17 (`TX2`)** (Controls CH4: Projector)

> [!IMPORTANT]
> **Active-LOW Logic:** Most 5V relay modules are **ACTIVE-LOW** (writing `LOW` turns the relay ON, writing `HIGH` turns it OFF). The firmware sets `const bool RELAY_ACTIVE_LOW = true;` to handle this automatically.

#### B. Daisy-Chaining the 5V Supply to `COM` Terminals:
The yellow jumper on the board connects `JD-VCC` to `VCC` for relay coil power, but the screw terminal `COM` ports are **electrically isolated**. You must bridge the 5V rail across all 4 channels:

```
 ESP32 VIN (5V) ──► K1 COM ──► K2 COM ──► K3 COM ──► K4 COM
```

#### C. Appliance Load Connections:
* **CH1 (5V DC Cooling Fan):**
  * `K1 NO` $\rightarrow$ Fan Red (+) wire
  * Fan Black (-) wire $\rightarrow$ Common GND
  * **1N4007 Flyback Diode:** Connected directly across the fan terminals!
    * **Silver Stripe (Cathode):** Must face **Fan Red (+) / Relay NO**
    * **Plain Black (Anode):** Must face **Fan Black (-) / GND**
    * *(If backwards, it creates a dead short when the relay clicks).*
* **CH2 (Room Light):**
  * `K2 NO` $\rightarrow$ `[220Ω Resistor]` $\rightarrow$ Yellow LED Anode (+) $\rightarrow$ LED Cathode (-) $\rightarrow$ GND
* **CH3 (Air Conditioner):**
  * `K3 NO` $\rightarrow$ `[220Ω Resistor]` $\rightarrow$ Blue LED Anode (+) $\rightarrow$ LED Cathode (-) $\rightarrow$ GND
* **CH4 (Projector):**
  * `K4 NO` $\rightarrow$ `[220Ω Resistor]` $\rightarrow$ White LED Anode (+) $\rightarrow$ LED Cathode (-) $\rightarrow$ GND

---

### Step 7: I2C 16×2 LCD Screen

```
I2C Backpack Pin      ESP32 Pin
───────────────────────────────
GND             ───►  GND
VCC             ───►  VIN (5V)   ◄── (Must be 5V, not 3.3V!)
SDA             ───►  GPIO 21
SCL             ───►  GPIO 22
```

* **Contrast Trimpot:** Adjust the brass screw on the blue potentiometer on the back of the LCD until characters appear sharp and dark.
* **3-Page Display Rotation (Every 3s):**
  * Page 1: `Tchr: Present/Absent` | `Total: X  Stud: Y`
  * Page 2: `Temp: XX.X C` | `Unknown alerts: Z`
  * Page 3: `Fan: ON/OFF Lght: ON/OFF` | `AC: ON/OFF Proj: ON/OFF`
  * Alert banners pop up temporarily on card scans, intruder alerts, and room status changes.

---

### Step 8: Cloud Architecture & Dual-Core FreeRTOS

To prevent network stalls from freezing physical door and card sensors:

1. **Core 1 (Application Core):** Runs `loop()` at **< 1ms latency**. Directly handles RFID scanning, IR beam tracking, relay switching, and LCD updates. It **never makes blocking HTTPS network calls**.
2. **Core 0 (Protocol Core):** Runs `firebaseTask` in the background:
   * Sets `vTaskDelay(pdMS_TO_TICKS(1500))` between consecutive SSL calls so the lwIP stack cleanly tears down the TCP socket and frees its 30KB TLS memory buffer.
   * Syncs live state to `/classroom/live.json`.
   * Pushes security alarms to `/classroom/alerts.json`.
   * Logs periodic history snapshots to `/classroom/history.json`.
3. **Mobile Hotspot Reliability:** Do **not** call `WiFi.config` after `WiFi.begin`, as it breaks the lwIP default gateway routing table. Use native DHCP and allow 4 seconds after boot for network stack stabilization.

---

## ⚠️ Real-World Troubleshooting & Gotchas Discovered

| Issue Encountered | Root Cause | Solution Verified |
| :--- | :--- | :--- |
| **Relay clicks, but fan doesn't spin** | 1N4007 flyback diode connected backwards, shorting 5V to GND | Flip diode: **Silver stripe (Cathode) to Fan Red (+)**, plain black to GND |
| **All devices behave in opposite** | Physical relay module is Active-LOW | Set `const bool RELAY_ACTIVE_LOW = true;` in sketch |
| **Can't find GPIO 17 on board** | Labeled as `TX2` on 30-pin ESP32 DevKit | Connect Relay IN4 to pin labeled **`TX2`** (next to D5) |
| **K2, K3, K4 don't turn on** | Screw terminal COMs are electrically isolated | Daisy-chain jumper wires: `K1 COM -> K2 COM -> K3 COM -> K4 COM` to 5V |
| **Buzzer is silent or clicks faintly** | Active buzzer driven with `tone()` or sticker on hole | Use `digitalWrite(HIGH/LOW)` instead of `tone()`, and peel off the top sticker |
| **Buzzer makes noise when idle** | Driving buzzer low-side from 5V into 3.3V GPIO | Connect Buzzer `(+)` to **GPIO 25**, `(-)` to **GND** for 100% idle silence |
| **Phantom card taps at startup** | Floating Wokwi simulator button pins (GPIO 15, 2, 16, 12, 34) | Disable `updateCards()` in code; rely 100% on real MFRC522 RFID reader |
| **Sensors lag & miss door crossings** | Synchronous `firebaseSyncLive()` blocked main loop for 3s | Offload Firebase to **Core 0 FreeRTOS background task** |
| **`[FIREBASE PUT ERR] connection refused`** | Calling `WiFi.config` after `WiFi.begin` broke lwIP gateway route | Remove post-connect `WiFi.config`, add 1.5s delay between SSL requests |
| **LCD dims when all devices turn ON** | 5V rail sags from ~850mA load exceeding PC USB 500mA limit | Power via **5V / 2A phone wall charger** or Expansion Board DC barrel jack |

---

## 🚀 Pre-Demo Verification Checklist

- [ ] **Power Supply:** ESP32 plugged into a 5V / 2A wall charger (not a 500mA USB port).
- [ ] **Diode Orientation:** 1N4007 silver stripe points to Fan Red (+) / Relay NO.
- [ ] **Relay COMs:** Middle screw terminals K1, K2, K3, K4 daisy-chained to ESP32 VIN (5V).
- [ ] **Relay Logic:** `RELAY_ACTIVE_LOW = true;` in [sketch.ino](file:///d:/Study/3.2/CSE-315/Project/simulation/wokwi_esp32_simulation/sketch.ino).
- [ ] **Buzzer:** `(+)` to GPIO 25, `(-)` to GND, protective top sticker removed.
- [ ] **RFID Module:** Powered by **3.3V ONLY**.
- [ ] **Dual IR Gate:** IR1 (GPIO 32) outside door, IR2 (GPIO 33) inside door, trimpots tuned to ~20cm.
- [ ] **LCD Screen:** Powered by 5V (VIN), SDA on 21, SCL on 22, contrast trimpot tuned.
- [ ] **Wi-Fi & Cloud:** SSID and Password set; Serial Monitor displays `[FIREBASE POST OK] -> HTTP 200`.
- [ ] **Mobile App:** Android App opens and displays live attendance, temperature gauge, and appliance states.

---

*Smart Classroom Management and Automation System — Team 06, CSE 315 / 316 Lab, University of Asia Pacific*
