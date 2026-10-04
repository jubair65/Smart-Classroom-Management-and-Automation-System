# Smart Classroom – Tinkercad Circuits Build Guide
CSE 316 · Team 06 · Section B1

Tinkercad can't import a circuit file, so you build it by hand (about 15–20 minutes) and paste in `smart_classroom.ino`.

## 1. What changes from the ESP32 proposal
| Proposal | Tinkercad version |
|---|---|
| ESP32 | Arduino Uno R3 |
| RFID RC522 + cards | 5 pushbuttons: Teacher, Student 1, 2, 3, Unknown card (or Serial: `T S1 S2 S3 U`) |
| 2 IR sensors | 2 pushbuttons (pressed = beam broken) (or Serial: `IN` / `OUT`) |
| DHT11/22 | TMP36 temperature sensor (click it to change the temperature) |
| 16×2 I2C LCD | LCD 16×2 (I2C) |
| Relay + fan / light / AC / projector | 4 relays driven by NPN transistors: Fan = DC motor, Light = yellow LED, AC = blue LED, Projector = white LED |
| Buzzer | Piezo |
| Wi-Fi, Firebase, mobile app | Not available. `CLOUD> {json}` records are printed to the Serial Monitor instead, showing exactly what would be uploaded. Timestamps are uptime. |

## 2. Parts list (Tinkercad component names)
1× Arduino Uno R3 · 1× Breadboard (full) · 1× LCD 16×2 (I2C) · 1× Temperature Sensor [TMP36] · 7× Pushbutton · 1× Piezo · 4× Relay SPDT · 4× NPN Transistor · 4× Diode (1N4007) · 4× Resistor 1 kΩ · 3× LED (yellow, blue, white) with 3× Resistor 220 Ω · 1× DC Motor

## 3. Wiring
Connect the Uno 5V and GND to the breadboard power rails first.

**LCD (I2C):** GND → GND, VCC → 5V, SDA → A4, SCL → A5.

**TMP36:** left pin → 5V, middle pin → A0, right pin → GND (flat side facing you).

**Buttons** (one leg to the Arduino pin, the opposite leg to GND; the code uses internal pull-ups, so no resistors are needed):
| Button | Pin |
|---|---|
| IR1 (outside) | D2 |
| IR2 (inside) | D3 |
| Teacher card | D4 |
| Student 1 card | D5 |
| Student 2 card | D6 |
| Student 3 card | D7 |
| Unknown card | D8 |

**Piezo:** + → D9, − → GND.

**Relay channels** (repeat for each; in the diagram the NPN is shown as C / B / E):
| Channel | Arduino pin | Load on relay NO |
|---|---|---|
| Fan | D10 | DC motor |
| Light | D11 | yellow LED + 220 Ω |
| AC | D12 | blue LED + 220 Ω |
| Projector | D13 | white LED + 220 Ω |

For each channel:
1. Arduino pin → 1 kΩ → transistor **base**.
2. Transistor **emitter** → GND.
3. Transistor **collector** → relay coil pin A.
4. Relay coil pin B → 5V.
5. Diode across the coil: cathode (striped end) to 5V, anode to the collector.
6. Relay **COM** → 5V.
7. Relay **NO** → load → GND. For the LEDs this is NO → 220 Ω → LED anode, cathode → GND. For the motor it is NO → motor pin 1, motor pin 2 → GND.

## 4. Code
1. Click **Code** → switch the mode to **Text** and delete the default code.
2. Paste the contents of `smart_classroom.ino`.
3. Click **Start Simulation**, then open the **Serial Monitor** (bottom of the code panel).

## 5. Using it
- **Enter:** press a card button, then press IR1 and IR2 in that order. Press IR1 first and IR2 within 3 s.
- **Exit:** press the card button again, then IR2 and IR1 in that order.
- **Unknown person:** press IR1 then IR2 with no card tap, or press the Unknown card button. The buzzer sounds, the LCD shows an alert, and `ALERT>` is logged. The person is not counted.
- **Serial shortcuts:** type `S1` and Send, then `IN` and Send. Other commands are `OUT`, `T`, `S2`, `S3`, `U` and `STATUS`.
- **Temperature:** click the TMP36 and drag the slider. Fan turns on at 27 °C or more and AC at 30 °C or more, but only while someone is in the room.
- **LCD** rotates every 3 s: teacher and student count, then temperature and unknown alerts, then device status.

## 6. Test run
1. `T` → `IN`: teacher in. Light and projector ON.
2. `S1` → `IN`, `S2` → `IN`: student count 2.
3. TMP36 to 28 °C: fan ON. TMP36 to 31 °C: AC ON.
4. `IN` with no card: unknown alert and buzzer, count stays 2.
5. `S1` → `OUT`, `S2` → `OUT`, `T` → `OUT`: all devices turn OFF.

## 7. Troubleshooting
- **LCD blank:** click the LCD, check its I2C address, and match the constructor `Adafruit_LiquidCrystal lcd(0);` (0 means address 0x20, 1 means 0x21, and so on).
- **Relay not clicking:** check the transistor orientation (collector to coil) and the diode direction.
- **Temperature reads oddly:** check the TMP36 orientation. Flat side facing you, pins read 5V, signal, GND from left to right.
- **Serial command ignored:** commands are not case-sensitive. A short pause ends the command if no newline is sent.
