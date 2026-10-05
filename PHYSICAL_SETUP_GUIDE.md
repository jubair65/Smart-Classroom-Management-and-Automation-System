# 🏫 Smart Classroom Management System — Physical Setup Guide

**Team 06 — CSE 315 / 316 (University of Asia Pacific)**  
*ESP32 + Firebase + Android App — Complete Hardware Build Manual*

---

## 📐 Physical Hardware Layout

![Smart Classroom Hardware Layout](hardware_layout.jpg)

> [!NOTE]
> The diagram above shows the complete physical wiring layout.
> **Red** = VCC/Power, **Black** = GND, **Blue** = SPI Data, **Green** = I2C / Sensors, **Orange** = Relay control signals.

---

## 🧩 Components List & Estimated Prices (Bangladesh Market)

> [!NOTE]
> Prices are approximate BDT values from Dhaka markets (Elephant Road / Banani) or online stores (Techshopbd.com, Robu.in).
> Prices are subject to change. Always verify before purchasing.

| # | Component | Specification | Qty | Unit Price (BDT) | Total (BDT) | Notes |
|---|-----------|--------------|-----|-------------------|-------------|-------|
| 1 | **ESP32 DevKit V1 (30-pin)** | 30-pin (NodeMCU-32S style), Dual-Core, Wi-Fi + BT | 1 | 350 – 420 | ~380 | Main microcontroller. 30-pin version fits better, is readily available, and avoids unusable SPI flash pins |
| 2 | **ESP32 30-Pin Expansion Board** | 30-pin socket breakout, screw terminals / G-V-S pins, DC jack | 1 | 180 – 250 | ~200 | **Highly Recommended** — Breaks out all GPIOs with dedicated VCC & GND. Eliminates messy loose wiring! |
| 3 | **MFRC522 RFID Module** | 13.56 MHz SPI, with antenna | 1 | 130 – 160 | ~150 | Usually comes bundled with 2 RFID tags (cards/keychain) |
| 4 | **RFID Cards / Key Tags** | ISO 14443A Mifare 1K | 4–6 | 20 – 30 ea | ~100 | One for Teacher, three for Students |
| 5 | **DHT22 Sensor** | Temp + Humidity, ±0.5°C accuracy | 1 | 120 – 160 | ~140 | Do NOT use DHT11 — too inaccurate and slow |
| 6 | **16×2 LCD with I2C Module** | HD44780 + PCF8574 I2C backpack | 1 | 150 – 200 | ~175 | Buy WITH the I2C backpack pre-soldered. Saves 12 wires |
| 7 | **Active Buzzer** | 5V, self-oscillating (not passive) | 1 | 15 – 25 | ~20 | MUST be active buzzer. Passive will NOT work with tone() |
| 8 | **4-Channel Relay Module** | 5V coil, 10A 250VAC contacts | 1 | 120 – 180 | ~150 | Single 4-channel board is easier than 4 separate relays |
| 9 | **IR Obstacle Sensor (x2)** | TCRT5000 style, adjustable range | 2 | 50 – 80 ea | ~130 | One for "Outside" door, one for "Inside" door |
| 10 | **5V DC Cooling Fan** | 5V 2-pin PC case fan | 1 | 80 – 150 | ~100 | ✅ Real fan for prototype — connected to Relay CH1 |
| 11 | **1N4007 Flyback Diode** | 1A, 1000V reverse blocking | 1 | 2 – 5 | ~5 | **CRITICAL** — must be wired across fan motor terminals |
| 12 | **Mini Breadboard** | 170 tie-points (SYB-170) or 400 tie-points | 1 | 30 – 50 | ~40 | Placed alongside expansion board to hold the 3 LEDs and 220Ω resistors |
| 13 | **Jumper Wires (Assorted)** | M-M, M-F, F-F 20cm packs (20 pcs ea) | 1 pack ea | 55 ea (165 total) | ~165 | **All 3 types needed**: M-M for Relay-to-Breadboard & tie-point links; F-F for sensor headers; M-F for expansion-to-breadboard |
| 14 | **USB Micro-B Cable** | Data cable for ESP32 programming | 1 | 50 – 80 | ~60 | Good quality data cable (NOT charge-only) |
| 15 | **5V 2A / 9V–12V 1A DC Adapter** (optional) | DC Barrel Jack or USB-A wall adapter | 1 | 80 – 150 | ~120 | Powers the expansion board DC jack or ESP32 without laptop connected |
| 16 | **10kΩ Resistor** | 1/4W, for GPIO 34 external pull-up | 1 | ~1 | ~2 | Required for Unknown Card button on GPIO 34 |
| 17 | **220Ω Resistors** | Current limiting for prototype LEDs | 3 | ~1 ea | ~5 | ✅ Required — one per LED (Light, AC, Projector) |
| 18 | **LED Yellow** | 5mm, for Light prototype | 1 | 3 – 5 | ~5 | ✅ Required — Relay CH2 (Light) indicator |
| 19 | **LED Blue** | 5mm, for AC prototype | 1 | 3 – 5 | ~5 | ✅ Required — Relay CH3 (AC) indicator |
| 20 | **LED White** | 5mm, for Projector prototype | 1 | 3 – 5 | ~5 | ✅ Required — Relay CH4 (Projector) indicator |
| 21 | **Cardboard / Wood Frame** | For door frame mock-up | 1 | 20 – 50 | ~40 | Mounting IR sensors on a door frame |

### 💰 Total Estimated Budget (Your Confirmed Build)

| Scenario | Cost (BDT) |
|----------|------------|
| **Your Build (30-pin ESP32 + Expansion Board + Mini Breadboard + DC Fan + 3 LEDs + all components)** | **1,650 – 1,950** |
| With door frame enclosure | 1,700 – 2,000 |
| With dedicated 9V/12V or 5V power adapter | 1,750 – 2,100 |

> [!TIP]
> **Where to Buy in Dhaka:**
> - **Elephant Road / Haatirpool, Dhaka** — Physical electronics shops, good for same-day purchase
> - **Techshop BD** (techshopbd.com) — Online, good selection and delivery
> - **Robu.in** — Indian site, ships to BD; good for DHT22 and RFID modules
> - **AliExpress** — Cheapest, but 2–4 weeks delivery

---

## 🔌 Complete Pin Wiring Table

### ESP32 GPIO Pin Assignment

| ESP32 GPIO | Peripheral | Pin on Module | Wire Color |
|------------|-----------|---------------|------------|
| **3.3V** | MFRC522 VCC | 3.3V | Red |
| **3.3V** | DHT22 VCC | Pin 1 | Red |
| **3.3V** | IR Sensor 1 VCC | VCC | Red |
| **3.3V** | IR Sensor 2 VCC | VCC | Red |
| **5V (VIN)** | LCD VCC | VCC | Red |
| **5V (VIN)** | Relay Module VCC | VCC | Red |
| **GND** | All modules GND | GND | Black |
| **GPIO 2** | Card Button S1 (Hasanul) | Button pin | Blue |
| **GPIO 4** | MFRC522 RST | RST | Purple |
| **GPIO 5** | MFRC522 SDA (SS/CS) | SDA | Orange |
| **GPIO 12** | Card Button S3 (Maria) | Button pin | Purple |
| **GPIO 13** | Relay CH3 — AC | IN3 | Blue |
| **GPIO 14** | Relay CH2 — Light | IN2 | Blue |
| **GPIO 15** | Card Button T (Teacher) | Button pin | Orange |
| **GPIO 16 (RX2)** | Card Button S2 (Jubair) | Button pin | Cyan |
| **GPIO 17 (TX2)** | Relay CH4 — Projector | IN4 | Blue |
| **GPIO 18** | MFRC522 SCK | SCK | Gray |
| **GPIO 19** | MFRC522 MISO | MISO | Gold |
| **GPIO 21** | LCD SDA (I2C) | SDA | Green |
| **GPIO 22** | LCD SCL (I2C) | SCL | Yellow |
| **GPIO 23** | MFRC522 MOSI | MOSI | Blue |
| **GPIO 25** | Active Buzzer (+) | + | Purple |
| **GPIO 26** | Relay CH1 — Fan | IN1 | Blue |
| **GPIO 27** | DHT22 Data | Pin 2 | Green |
| **GPIO 32** | IR Sensor 1 OUT (Outside/Entry) | OUT | Green |
| **GPIO 33** | IR Sensor 2 OUT (Inside/Exit) | OUT | Red |
| **GPIO 34** | Unknown Card Button | Button pin | Gray |

---

## 🛠️ Step-by-Step Physical Assembly Instructions

### Phase 1: ESP32 Expansion Board & Power Distribution Setup

1. **Mount the ESP32 on the Expansion Board:**
   - Align the 30-pin ESP32 DevKit V1 with the socket on the **30-pin ESP32 Expansion Board** (antenna pointing outwards, USB port accessible).
   - Press down gently and evenly until all pins are fully seated in the headers.
2. **Expansion Board Power Distribution:**
   - The expansion board provides dedicated **VCC** (5V / 3.3V selectable or separated rails) and **GND** terminal pins for every GPIO.
   - For **5V peripherals** (LCD VCC, Relay VCC, DC Fan): connect to **5V / VIN** terminals.
   - For **3.3V peripherals** (MFRC522 VCC, DHT22, IR Sensors): connect to **3.3V** terminals.
3. **Mini Breadboard Setup (For Discrete LEDs & Resistors):**
   - Place the **170-point mini breadboard** next to the expansion board.
   - Use it exclusively to hold the 3 discrete prototype LEDs (Yellow, Blue, White) and their 220Ω resistors.
   - Connect the mini breadboard's ground rail to a **GND** terminal on the expansion board using an M-F jumper wire.

> [!IMPORTANT]
> The ESP32 has TWO voltage outputs:
> - **3.3V** — for sensors: RFID module, DHT22, IR sensors
> - **VIN (5V)** — for the LCD and Relay module
>
> **Never connect the MFRC522 RFID module to 5V. It will be permanently damaged.**

---

### Phase 2: MFRC522 RFID Reader (SPI Bus)

```
MFRC522 Pin    →  ESP32 GPIO
──────────────────────────────
3.3V (VCC)     →  3.3V
GND            →  GND
RST            →  GPIO 4    (Purple wire)
SDA (SS/CS)    →  GPIO 5    (Orange wire)
SCK            →  GPIO 18   (Gray wire)
MISO           →  GPIO 19   (Gold wire)
MOSI           →  GPIO 23   (Blue wire)
```

> [!TIP]
> Most MFRC522 boards come with unsoldered header pins. **Solder all 8 pins before wiring.** Keep SPI wires shorter than 20cm to avoid signal integrity issues.

---

### Phase 3: DHT22 Temperature & Humidity Sensor

```
DHT22 Pin   →  ESP32
────────────────────────
Pin 1 (+)   →  3.3V
Pin 2 (Data)→  GPIO 27   (Green wire)
Pin 3 (NC)  →  (not connected)
Pin 4 (-)   →  GND
```

> [!NOTE]
> The DHT22 **module** (with the small PCB backboard) already has the 10kΩ pull-up resistor built in. If you have a **bare DHT22 sensor** (without PCB), add a 10kΩ resistor between Data pin and 3.3V.

---

### Phase 4: 16×2 LCD with I2C Module

The I2C backpack (PCF8574) reduces wiring from 12 wires to just 4:

```
I2C LCD Pin  →  ESP32
───────────────────────
GND          →  GND
VCC          →  5V (VIN)
SDA          →  GPIO 21   (Green wire)
SCL          →  GPIO 22   (Yellow wire)
```

> [!TIP]
> **I2C Address**: Most I2C LCD modules default to `0x27`. If the LCD stays blank after uploading, try `0x3F`. You can run an I2C scanner sketch to find the address. Address is set in code: `LiquidCrystal_I2C lcd(0x27, 16, 2);`

---

### Phase 5: Active Buzzer

```
Buzzer Pin        →  ESP32
───────────────────────────
+ (longer leg)    →  GPIO 25
- (shorter leg)   →  GND
```

> [!CAUTION]
> Use an **ACTIVE** buzzer only (it has a built-in oscillator). A **passive** buzzer (just a piezo disk) will not produce expected alarm sounds with the `tone()` function. Active buzzers are usually marked with a sticker covering the hole on top.

---

### Phase 6: 4-Channel Relay Module — Control Wiring

**Step 1 — Connect relay control pins to ESP32:**
```
Relay Pin  →  ESP32 GPIO
────────────────────────────────────────────────
VCC        →  5V (VIN)
GND        →  GND
IN1        →  GPIO 26   ← CH1: DC Fan (real motor)
IN2        →  GPIO 14   ← CH2: Yellow LED (Light prototype)
IN3        →  GPIO 13   ← CH3: Blue LED (AC prototype)
IN4        →  GPIO 17   ← CH4: White LED (Projector prototype)
```

---

### Phase 6a: CH1 — DC Fan Load Wiring (Real Motor)

The fan is switched via the relay using the ESP32's own 5V (VIN) rail:

```
ESP32 VIN (5V) (+) ──────────► Relay CH1 COM
                               Relay CH1 NO ──────────► Fan (+) Red wire
                                                              │
                                                         [1N4007 diode]
                                                         Cathode (stripe)→ Fan (+)
                                                         Anode           → Fan (-)
ESP32 GND (-)  ──────────────────────────────────────► Fan (-) Black wire
```

> [!WARNING]
> **CRITICAL — 1N4007 Flyback Diode (Fan only — do NOT skip this):**
>
> The DC fan motor produces a large back-EMF voltage spike when the relay cuts power. This can reset or permanently damage the ESP32.
>
> Wire the diode **across the fan motor terminals** (not across the relay):
> - **Cathode (Silver stripe)** → Fan (+) / Relay NO side
> - **Anode (plain black end)** → Fan (−) / GND
>
> ⚠️ If wired **backwards**: it short-circuits your 5V supply the moment the relay turns ON.

---

### Phase 6b: CH2 / CH3 / CH4 — LED Load Wiring (Light / AC / Projector prototypes)

Each LED channel uses the same simple circuit. Wire the LED through a **220Ω resistor** to limit current:

```
Relay CHx NO ──────────► [220Ω Resistor] ──────────► LED (+) Anode (longer leg)
                                                       LED (-) Cathode (shorter leg) ──► GND
Relay CHx COM ──────────► ESP32 VIN (5V)
```

**Channel-by-channel LED assignment:**

| Relay Channel | GPIO | LED Color | Represents |
|--------------|------|-----------|------------|
| CH2 | GPIO 14 | 🟡 Yellow | Room Light |
| CH3 | GPIO 13 | 🔵 Blue | Air Conditioner (AC) |
| CH4 | GPIO 17 | ⚪ White | Projector |

> [!NOTE]
> LEDs do NOT need a flyback diode — they are not inductive loads. The 220Ω resistor is sufficient protection.
> Each LED draws ~15mA at 5V through a 220Ω resistor, well within safe limits.

---

### Phase 7: IR Door Sensors

Mount two IR sensors on the door frame, approximately 10–15cm apart:

```
IR Sensor 1 (OUTSIDE face of door):    IR Sensor 2 (INSIDE face of door):
VCC → 3.3V                              VCC → 3.3V
GND → GND                               GND → GND
OUT → GPIO 32                           OUT → GPIO 33
```

> [!TIP]
> Adjust the small blue **potentiometer (trim pot)** on each IR sensor board with a screwdriver to set the detection range (~20–40cm for a doorway).
>
> **Direction detection & Presence logic:**
> - **Person ENTERING**: Tap RFID card outside the door, then step through `IR1 -> IR2`. Increments `totalCount`.
> - **Person EXITING (Contactless Free Exit)**: No RFID tap required! Simply step through `IR2 -> IR1`. Decrements `totalCount`.
> - **Teacher Presence Rule**:
>   - If `totalCount > 0` $\rightarrow$ Teacher is **PRESENT**, appliances active.
>   - If `totalCount == 0` $\rightarrow$ Teacher is **ABSENT**, all appliances (Fan, Light, AC, Projector) automatically shut OFF.

---

### Phase 8: RFID Card Registration

The firmware uses hardcoded UIDs. To register your physical RFID cards:

1. Upload the sketch, open **Serial Monitor** at `115200 baud`.
2. Tap any RFID card on the MFRC522 reader.
3. Serial output will show: `[RFID SCAN] Card UID: XXXXXXXX`
4. Open `sketch.ino`, update lines ~404–408:
   ```cpp
   if (scannedHex.indexOf("DEADBEEF") >= 0)      onCard(0); // Teacher
   else if (scannedHex.indexOf("12345678") >= 0) onCard(1); // Student 1
   else if (scannedHex.indexOf("AABBCCDD") >= 0) onCard(2); // Student 2
   else if (scannedHex.indexOf("11223344") >= 0) onCard(3); // Student 3
   ```
   Replace the UID strings with your actual card UIDs.
5. Re-upload the updated sketch.

---

### Phase 9: Wi-Fi & Firebase Configuration (Physical ESP32)

1. Open `sketch.ino`, update lines 20–21:
   ```cpp
   const char* WIFI_SSID = "YourHotspotName";  // 2.4GHz Wi-Fi only
   const char* WIFI_PASS = "YourPassword";
   ```
2. Firebase URL remains the same as used in Wokwi:
   ```cpp
   const char* FIREBASE_URL = "https://smart-classroom-team06-default-rtdb.asia-southeast1.firebasedatabase.app";
   ```
3. Upload the sketch to the physical ESP32.
4. Open Serial Monitor — you should see: `Connected! IP: 192.168.x.x`
5. Open your Android app and log in — dashboard should update in real time!

---

## ⚠️ Common Mistakes to Avoid

### Critical Errors (Will damage hardware or cause non-functional system)

| # | Mistake | Problem | Fix |
|---|---------|---------|-----|
| 1 | **Powering MFRC522 from 5V** | Module permanently burned; SPI errors | Always use **3.3V** for MFRC522 |
| 2 | **No flyback diode on DC motor fan** | ESP32 resets randomly or gets damaged | Add **1N4007 diode** across fan terminals |
| 3 | **Flyback diode installed backwards** | Short-circuits power supply when relay turns ON | Silver stripe (Cathode) must go to Fan (+) |
| 4 | **Using DHT11 instead of DHT22** | 1°C resolution makes 27.0°C / 25.5°C thresholds meaningless | Use DHT22 specifically |
| 5 | **Using a passive buzzer** | Buzzer produces no sound or very faint noise | Use an **active buzzer** with built-in oscillator |
| 6 | **GPIO 34 used with INPUT_PULLUP** | GPIO 34/35/36/39 have NO internal pull-up on ESP32 | Add external **10kΩ** resistor from GPIO 34 to 3.3V |

---

### Software / Configuration Mistakes

| # | Mistake | Problem | Fix |
|---|---------|---------|-----|
| 7 | **Wrong I2C address for LCD** | LCD stays dark or shows garbage | Default is `0x27`; try `0x3F`; run I2C scanner |
| 8 | **Firebase URL has trailing slash** | HTTP PUT returns 404 error | Remove trailing `/` from URL |
| 9 | **Firebase rules not in Test Mode** | All writes return 401 Unauthorized | Set rules: `".read": true, ".write": true` |
| 10 | **Using charge-only USB cable** | Cannot upload sketch; device not detected | Use a **data-capable** Micro-B USB cable |
| 11 | **Wrong baud rate in Serial Monitor** | Garbled text in Serial output | Set Serial Monitor to exactly **115200 baud** |
| 12 | **RFID card UID not updated in code** | Every tap triggers Unknown card alert | Read actual UID from Serial Monitor; update sketch |
| 13 | **5GHz Wi-Fi used** | ESP32 stays offline; never connects | ESP32 **only supports 2.4GHz** Wi-Fi networks |

---

### Wiring / Assembly Mistakes

| # | Mistake | Problem | Fix |
|---|---------|---------|-----|
| 14 | **IR sensors wired to 5V** | Logic-level mismatch on 3.3V input pins | Use **3.3V** for IR sensors |
| 15 | **SPI wires MOSI/MISO swapped** | RFID never detects cards | Verify: MOSI=23, MISO=19, SCK=18 |
| 16 | **IR1 and IR2 sensors swapped** | Entry detected as exit and vice versa | IR1 (GPIO 32) = OUTSIDE; IR2 (GPIO 33) = INSIDE |
| 17 | **Relay IN logic inverted** | Some relay boards are ACTIVE LOW | If relay triggers on LOW, swap `HIGH : LOW` in code |
| 18 | **LCD SDA/SCL wires swapped** | I2C fails; LCD blank | SDA → GPIO 21, SCL → GPIO 22 |
| 19 | **Fan connected directly to GPIO** | GPIO max 12mA; motor draws much more; pin burns | Always switch fan power via **relay module** |
| 20 | **Long SPI wires for RFID (>30cm)** | Intermittent RFID reads; cards missed | Keep MFRC522 wires under **20cm** |

---

## 🧪 Pre-Power-On Safety Checklist

Before plugging in the USB cable for the first time:

- [ ] ESP32 is firmly seated in the 30-pin expansion board socket with correct orientation
- [ ] All **3.3V** peripherals (RFID, DHT22, IR sensors) are on **3.3V**, NOT 5V
- [ ] All **5V** peripherals (LCD, Relay VCC) are on **5V (VIN)**, NOT 3.3V
- [ ] **GPIO 34** has the 10kΩ external pull-up resistor to 3.3V
- [ ] **MFRC522 SPI wires**: SCK=18, MISO=19, MOSI=23, SS=5, RST=4
- [ ] **I2C wires**: SDA=21, SCL=22
- [ ] **DHT22** Data pin on GPIO 27
- [ ] **Relay IN pins**: IN1=GPIO 26, IN2=GPIO 14, IN3=GPIO 13, IN4=GPIO 17
- [ ] **IR sensors**: OUT1=GPIO 32 (outside), OUT2=GPIO 33 (inside)
- [ ] **Buzzer (+)** on GPIO 25
- [ ] **1N4007 diode** across fan motor with correct polarity (stripe to Fan+)
- [ ] No bare wires are touching each other (especially near power rails)

---

## 📡 Complete System Architecture

```
                 ┌─────────────────────────────────────────────────┐
                 │          Smart Classroom System                 │
                 └──────────────────┬──────────────────────────────┘
                                    │
       ┌────────────────────────────┼─────────────────────────────┐
       │                           │                             │
┌──────▼──────┐           ┌────────▼────────┐           ┌───────▼──────┐
│  Sensors &   │           │  ESP32 DevKit   │           │  4 Actuators │
│  RFID Input  │           │  (Main Brain)   │           │  via Relays  │
│             │           │                 │           │             │
│ MFRC522     │──SPI────►│ WiFi to Cloud   │──GPIO──►  │ Fan (CH1)   │
│ DHT22       │──GPIO───►│ Direction Logic  │           │ Light (CH2) │
│ IR1/IR2     │──GPIO───►│ Firebase REST    │           │ AC (CH3)    │
│ Buzzer      │◄──GPIO───│ LCD Rendering    │           │ Proj (CH4)  │
│ LCD 16x2    │◄──I2C────│                 │           │             │
└─────────────┘           └────────┬────────┘           └─────────────┘
                                   │ HTTPS REST
                                   ▼
                       ┌─────────────────────┐
                       │  Firebase Realtime  │
                       │     Database        │
                       │                     │
                       │ /classroom/live      │
                       │ /classroom/alerts    │
                       │ /classroom/history   │
                       └──────────┬──────────┘
                                  │ WebSocket
                                  ▼
                       ┌─────────────────────┐
                       │  Android App        │
                       │ (Kotlin + MVVM)     │
                       │                     │
                       │ Login (Auth)         │
                       │ Live Dashboard       │
                       │ Intruder Alerts      │
                       │ History Log          │
                       └─────────────────────┘
```

---

## 🚀 Automation Rules Quick Reference

| Condition | Fan | Light | AC | Projector |
|-----------|-----|-------|----|-----------|
| Room empty (no one inside) | OFF | OFF | OFF | OFF |
| Students only (no teacher) | Temp-based | ON | Temp-based | OFF |
| Teacher only | Temp-based | ON | Temp-based | ON |
| Teacher + Students | Temp-based | ON | Temp-based | ON |

**Temperature Thresholds:**
- Fan ON at >= 27.0°C | Fan OFF at <= 25.5°C
- AC ON at >= 30.0°C  | AC OFF at <= 28.5°C

---

## 🔗 Related Project Files

| File | Description |
|------|-------------|
| [`3D_PROTOTYPE_PLACEMENT_GUIDE.md`](file:///d:/Study/3.2/CSE-315/Project/simulation/3D_PROTOTYPE_PLACEMENT_GUIDE.md) | 3D prototype spatial placement guide with renders & dimensional zoning |
| [`3d_prototype_viewer/index.html`](file:///d:/Study/3.2/CSE-315/Project/simulation/3d_prototype_viewer/index.html) | Interactive 3D Web Prototype Viewer (Three.js with 360° rotation) |
| [`RoboticsBD_Components_Price_List.xlsx`](file:///d:/Study/3.2/CSE-315/Project/simulation/RoboticsBD_Components_Price_List.xlsx) | Excel BOM Spreadsheet with live prices & purchase links from store.roboticsbd.com |
| [`RoboticsBD_Components_Price_List.csv`](file:///d:/Study/3.2/CSE-315/Project/simulation/RoboticsBD_Components_Price_List.csv) | CSV version of the RoboticsBD components price list |
| [`sketch.ino`](file:///d:/Study/3.2/CSE-315/Project/simulation/wokwi_esp32_simulation/sketch.ino) | Full ESP32 firmware with Firebase sync |
| [`diagram.json`](file:///d:/Study/3.2/CSE-315/Project/simulation/wokwi_esp32_simulation/diagram.json) | Wokwi circuit diagram (virtual reference) |
| [`README_WOKWI.md`](file:///d:/Study/3.2/CSE-315/Project/simulation/wokwi_esp32_simulation/README_WOKWI.md) | Wokwi simulation guide |
| [`PHASE_2_FIREBASE_GUIDE.md`](file:///d:/Study/3.2/CSE-315/Project/simulation/wokwi_esp32_simulation/PHASE_2_FIREBASE_GUIDE.md) | Firebase setup instructions |
| [`README_ANDROID.md`](file:///d:/Study/3.2/CSE-315/Project/simulation/android_app/README_ANDROID.md) | Android app build and run guide |

---

*Smart Classroom Management and Automation System — Team 06, CSE 316 Lab, University of Asia Pacific*
