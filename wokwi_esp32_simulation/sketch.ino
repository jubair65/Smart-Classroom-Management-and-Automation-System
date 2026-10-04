// ==============================================================================
// Smart Classroom Management and Automation System (ESP32 Edition)
// Team 06 - CSE 315 / 316 - University of Asia Pacific (UAP)
// Matches 100% of working.ino logic with ESP32 Hardware + Wokwi Simulation
// ==============================================================================

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>

// ---------------- FIREBASE & WI-FI CONFIGURATION ----------------
// For Wokwi simulation: SSID = "Wokwi-GUEST", Password = ""
// For physical ESP32: enter your mobile hotspot / home Wi-Fi credentials
const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASS     = "";

// Paste your Firebase Realtime Database URL (MUST start with https:// and have NO trailing slash)
// E.g.: "https://smart-classroom-team06-default-rtdb.firebaseio.com"
const char* FIREBASE_URL  = "https://smart-classroom-team06-default-rtdb.asia-southeast1.firebasedatabase.app";

// If Firebase rules are in Test Mode (.read: true, .write: true), leave auth empty ""
const char* FIREBASE_AUTH = "";

// ---------------- HARDWARE PIN DEFINITIONS (ESP32) ----------------
// LCD I2C Pins: SDA = GPIO 21, SCL = GPIO 22
LiquidCrystal_I2C lcd(0x27, 16, 2);

// RFID MFRC522 Pins (VSPI Hardware Bus)
#define PIN_SS          5   // SDA / SS (Chip Select)
#define PIN_RST         4   // Reset Pin
// Hardware SPI: SCK = 18, MISO = 19, MOSI = 23
MFRC522 rfid(PIN_SS, PIN_RST);

// DHT22 Temperature & Humidity Sensor
#define PIN_DHT         27
#define DHTTYPE         DHT22
DHT dht(PIN_DHT, DHTTYPE);

// Two IR Direction Obstacle Sensors (active LOW)
const uint8_t PIN_IR1   = 32;   // Outside sensor (Entry trigger)
const uint8_t PIN_IR2   = 33;   // Inside sensor  (Exit trigger)

// Pushbuttons for Card Taps (Teacher, S1, S2, S3, Unknown)
const uint8_t CARD_PINS[5] = { 15, 2, 16, 12, 34 };

// Active Buzzer
const uint8_t PIN_BUZ   = 25;

// 4-Channel Relays (HIGH = ON, LOW = OFF)
const uint8_t PIN_FAN   = 26;   // Channel 1: Ceiling Fan
const uint8_t PIN_LIGHT = 14;   // Channel 2: Room Light
const uint8_t PIN_AC    = 13;   // Channel 3: AC
const uint8_t PIN_PROJ  = 17;   // Channel 4: Projector (TX2)

// ---------------- SETTINGS & THRESHOLDS ----------------
const int FAN_ON_X10    = 270, FAN_OFF_X10 = 255; // 27.0 C, 25.5 C
const int AC_ON_X10     = 300, AC_OFF_X10  = 285; // 30.0 C, 28.5 C

const unsigned long TAG_WINDOW_MS = 10000UL; // 10s to cross door after card scan
const unsigned long IR_TIMEOUT_MS = 3000UL;  // 3s max between IR1 and IR2
const unsigned long ALERT_MS      = 2500UL;  // Alert message duration
const unsigned long PAGE_MS       = 3000UL;  // LCD page rotate
const unsigned long TEMP_MS       = 1000UL;  // Temperature check interval
const unsigned long DEBOUNCE_MS   = 50UL;    // Button debounce window

// ---------------- USER REGISTRY ----------------
struct Person {
  char name[16];
  bool isTeacher;
  bool inside;
};

Person people[4] = {
  { "Sayma Ma'am", true,  false }, // Teacher
  { "Hasanul",     false, false }, // Student 1
  { "Jubair",      false, false }, // Student 2
  { "Maria",       false, false }  // Student 3
};

// ---------------- SYSTEM STATE ----------------
int  tempX10 = 260; // 26.0 C default
int  totalCount = 0, studentCount = 0, unknownCount = 0;
bool teacherPresent = false;
bool fanOn = false, lightOn = false, acOn = false, projOn = false;

struct StudentEntry {
  uint8_t personIdx;
  char enterTime[9];
};
StudentEntry activeStudents[4];
uint8_t activeStudentCount = 0;

int8_t pendingIdx = -1;
unsigned long pendingTime = 0;

enum IRState { IR_IDLE, IR_A_FIRST, IR_B_FIRST, IR_WAIT_CLEAR };
IRState irState = IR_IDLE;
unsigned long irT = 0;
bool prevB1 = false, prevB2 = false;
bool prevCard[5] = { true, true, true, true, true };
unsigned long lastDebounce[5] = { 0, 0, 0, 0, 0 }; // Per-button debounce timestamps

char alertL1[17] = "", alertL2[17] = "";
unsigned long alertUntil = 0, lastPage = 0, lastRender = 0, lastTemp = 0;
unsigned long lastLiveSync = 0, lastHistorySync = 0;
uint8_t page = 0;
bool dirtyLive = true;
bool dirtyHistory = true;

char cmdBuf[12];
uint8_t cmdLen = 0;
unsigned long lastRx = 0;

// ---------------- HELPERS ----------------
void stamp(char* out, size_t maxLen = 9) {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 100)) {
    strftime(out, maxLen, "%H:%M:%S", &timeinfo);
  } else {
    unsigned long s = millis() / 1000UL;
    snprintf(out, maxLen, "%02lu:%02lu:%02lu", (s / 3600UL) % 100UL, (s / 60UL) % 60UL, s % 60UL);
  }
}

void stampDate(char* out, size_t maxLen = 12) {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 100)) {
    strftime(out, maxLen, "%Y-%m-%d", &timeinfo);
  } else {
    strncpy(out, "1970-01-01", maxLen); // NTP unavailable — neutral fallback
    out[maxLen - 1] = '\0';
  }
}

void lcdLine(uint8_t row, const char* s) {
  char b[17];
  snprintf(b, sizeof(b), "%-16s", s);
  lcd.setCursor(0, row);
  lcd.print(b);
}

void beep(unsigned int ms, unsigned int freq) {
  tone(PIN_BUZ, freq, ms); // tone() stops itself after ms via hardware timer — non-blocking
}

void alarmBeep() {
  // Intentionally blocking — security alert must be fully heard
  for (uint8_t i = 0; i < 3; i++) {
    tone(PIN_BUZ, 2000, 150);
    delay(250); // 150ms tone + 100ms gap
  }
}

void showAlert(const char* l1, const char* l2, unsigned long ms = ALERT_MS) {
  strncpy(alertL1, l1, 16); alertL1[16] = '\0';
  strncpy(alertL2, l2, 16); alertL2[16] = '\0';
  alertUntil = millis() + ms;
  lastRender = 0;
}

// ---------------- FIREBASE REST CLIENT METHODS ----------------
void firebasePut(const char* path, const char* jsonPayload) {
  if (WiFi.status() != WL_CONNECTED || strlen(FIREBASE_URL) < 12) return;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  https.setTimeout(3000);
  https.setReuse(false); // Do not keep socket alive on temporary stack client

  String url = String(FIREBASE_URL) + path;
  if (strlen(FIREBASE_AUTH) > 0) {
    url += "?auth=" + String(FIREBASE_AUTH);
  }

  if (https.begin(client, url)) {
    https.addHeader("Content-Type", "application/json");
    https.addHeader("Connection", "close");
    int code = https.PUT(jsonPayload);
    if (code > 0) {
      Serial.print(F("[FIREBASE PUT] -> HTTP ")); Serial.println(code);
    } else {
      Serial.print(F("[FIREBASE PUT ERR] ")); Serial.println(https.errorToString(code));
    }
    https.end();
  }
  client.stop(); // Immediately release lwIP socket and TCP buffer
}


void firebasePost(const char* path, const char* jsonPayload) {
  if (WiFi.status() != WL_CONNECTED || strlen(FIREBASE_URL) < 12) return;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  https.setTimeout(3000);
  https.setReuse(false); // Do not keep socket alive on temporary stack client

  String url = String(FIREBASE_URL) + path;
  if (strlen(FIREBASE_AUTH) > 0) {
    url += "?auth=" + String(FIREBASE_AUTH);
  }

  if (https.begin(client, url)) {
    https.addHeader("Content-Type", "application/json");
    https.addHeader("Connection", "close");
    int code = https.POST(jsonPayload);
    if (code > 0) {
      Serial.print(F("[FIREBASE POST] -> HTTP ")); Serial.println(code);
    } else {
      Serial.print(F("[FIREBASE POST ERR] ")); Serial.println(https.errorToString(code));
    }
    https.end();
  }
  client.stop(); // Immediately release lwIP socket and TCP buffer
}


void firebaseSyncLive() {
  char ts[9]; stamp(ts);
  char dt[12]; stampDate(dt);

  // Format students array in descending order of entry (most recently entered first)
  char studentsJson[260] = "[";
  for (int i = (int)activeStudentCount - 1; i >= 0; i--) {
    uint8_t pIdx = activeStudents[i].personIdx;
    char item[64];
    snprintf(item, sizeof(item), "{\"name\":\"%s\",\"enterTime\":\"%s\"}%s",
      people[pIdx].name,
      activeStudents[i].enterTime,
      (i > 0) ? "," : ""
    );
    strncat(studentsJson, item, sizeof(studentsJson) - strlen(studentsJson) - 1);
  }
  strncat(studentsJson, "]", sizeof(studentsJson) - strlen(studentsJson) - 1);

  char b[600];
  snprintf(b, sizeof(b),
    "{\"teacherPresent\":%s,\"teacherName\":\"%s\",\"studentCount\":%d,\"unknownCount\":%d,\"temperature\":%d.%d,\"fan\":%s,\"light\":%s,\"ac\":%s,\"projector\":%s,\"time\":\"%s\",\"date\":\"%s\",\"lastUpdated\":\"%s %s\",\"students\":%s}",
    teacherPresent ? "true" : "false",
    teacherPresent ? people[0].name : "",
    studentCount,
    unknownCount,
    tempX10 / 10,
    abs(tempX10 % 10),
    fanOn ? "true" : "false",
    lightOn ? "true" : "false",
    acOn ? "true" : "false",
    projOn ? "true" : "false",
    ts,
    dt,
    dt,
    ts,
    studentsJson
  );
  firebasePut("/classroom/live.json", b);
}

void firebasePushAlert(const char* type, const char* message) {
  char ts[9]; stamp(ts);
  char dt[12]; stampDate(dt);
  char b[340];
  snprintf(b, sizeof(b),
    "{\"type\":\"%s\",\"message\":\"%s\",\"date\":\"%s\",\"time\":\"%s\",\"students\":%d,\"unknownCount\":%d,\"teacherPresent\":%s}",
    type,
    message,
    dt,
    ts,
    studentCount,
    unknownCount,
    teacherPresent ? "true" : "false"
  );
  firebasePost("/classroom/alerts.json", b);
}

void firebasePushHistory() {
  char ts[9]; stamp(ts);
  char dt[12]; stampDate(dt);
  char b[340];
  snprintf(b, sizeof(b),
    "{\"date\":\"%s\",\"time\":\"%s\",\"teacher\":\"%s\",\"students\":%d,\"unknownCount\":%d,\"temp\":%d.%d,\"fan\":%d,\"light\":%d,\"ac\":%d,\"proj\":%d}",
    dt,
    ts,
    teacherPresent ? "PRESENT" : "ABSENT",
    studentCount,
    unknownCount,
    tempX10 / 10,
    abs(tempX10 % 10),
    fanOn ? 1 : 0,
    lightOn ? 1 : 0,
    acOn ? 1 : 0,
    projOn ? 1 : 0
  );
  firebasePost("/classroom/history.json", b);
}

void logEvent(const char* type, const char* detail, bool unknown) {
  char ts[9]; stamp(ts);
  Serial.print('['); Serial.print(ts); Serial.print(F("] "));
  Serial.print(type); Serial.print(F(" : ")); Serial.println(detail);
  if (unknown) {
    Serial.print(F("ALERT> Unknown person detected at ")); Serial.println(ts);
    firebasePushAlert("UNKNOWN_PERSON", detail);
  }
}



// ---------------- TEMPERATURE READING (DHT22) ----------------
void readTemperature() {
  float t = dht.readTemperature();
  if (!isnan(t)) {
    int newTemp = (int)(t * 10.0f);
    if (abs(newTemp - tempX10) >= 5) { // 0.5°C change triggers live sync, avoiding micro-jitter spam
      dirtyLive = true;
    }
    tempX10 = newTemp;
  }
}

// ---------------- AUTOMATION LOGIC (MATCHES working.ino) ----------------
void applyAutomation() {
  // Occupied if totalCount > 0
  bool occupied = (totalCount > 0);
  bool nLight   = occupied;
  bool nProj    = teacherPresent;   // Projector active only when teacher is present
  bool nFan     = fanOn;
  bool nAC      = acOn;

  if (!occupied) {
    // Empty classroom: shut everything down
    nFan   = false;
    nAC    = false;
    nLight = false;
    nProj  = false;
  } else {
    // Occupied classroom: fan and AC control based on temperature
    if (tempX10 >= FAN_ON_X10)       nFan = true;
    else if (tempX10 <= FAN_OFF_X10) nFan = false;

    if (tempX10 >= AC_ON_X10)        nAC = true;
    else if (tempX10 <= AC_OFF_X10)  nAC = false;
  }

  if (nLight != lightOn || nProj != projOn || nFan != fanOn || nAC != acOn) {
    dirtyLive = true;
    dirtyHistory = true;
  }
  lightOn = nLight;
  projOn  = nProj;
  fanOn   = nFan;
  acOn    = nAC;

  // Active-HIGH for Wokwi Relay Module & direct LED driving
  digitalWrite(PIN_LIGHT, lightOn ? HIGH : LOW);
  digitalWrite(PIN_PROJ,  projOn  ? HIGH : LOW);
  digitalWrite(PIN_FAN,   fanOn   ? HIGH : LOW);
  digitalWrite(PIN_AC,    acOn    ? HIGH : LOW);
}

// ---------------- RFID CARD HANDLING ----------------
void onCard(uint8_t idx) {
  char m[32];
  if (idx >= 4) {
    pendingIdx = -1;
    unknownCount++;
    showAlert("UNKNOWN CARD", "Access denied", ALERT_MS);
    alarmBeep();
    logEvent("UNKNOWN", "Unregistered card tapped", true);
    dirtyLive = true;
    dirtyHistory = true;
    firebaseSyncLive(); // Immediate live sync for instant mobile app alarm!
    lastLiveSync = millis();
    return;
  }

  // Bug fix: don't overwrite a still-valid pending tap (within TAG_WINDOW_MS)
  // Prevents a second card tap from "stealing" the door entry from the first person
  if (pendingIdx >= 0 && (millis() - pendingTime) <= TAG_WINDOW_MS) {
    showAlert("Wait — scan again", "after crossing!", 2000);
    Serial.println(F("[CARD] Ignored — previous tap still pending."));
    return;
  }

  pendingIdx = idx;
  pendingTime = millis();
  showAlert(people[idx].isTeacher ? "Teacher card OK" : "Student card OK", people[idx].name, 2000);
  beep(80, 1500);
  snprintf(m, sizeof(m), "%s tapped card", people[idx].name);
  logEvent("CARD", m, false);
}


// Read physical MFRC522 RFID reader
void checkMFRC522() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  String scannedHex = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) scannedHex += "0";
    scannedHex += String(rfid.uid.uidByte[i], HEX);
  }
  scannedHex.toUpperCase();

  Serial.print(F("[RFID SCAN] Card UID: "));
  Serial.println(scannedHex);

  if (scannedHex.indexOf("DEADBEEF") >= 0)      onCard(0); // Teacher (Sayma Ma'am)
  else if (scannedHex.indexOf("12345678") >= 0) onCard(1); // Student 1 (Hasanul)
  else if (scannedHex.indexOf("AABBCCDD") >= 0) onCard(2); // Student 2 (Jubair)
  else if (scannedHex.indexOf("11223344") >= 0) onCard(3); // Student 3 (Maria)
  else onCard(4); // Unknown Card

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// ---------------- DOOR PASSAGE DETECTION (IR SENSORS) ----------------
void handleCrossing(bool entering) {
  char m[40];

  if (entering) {
    bool tagValid = (pendingIdx >= 0) && ((millis() - pendingTime) <= TAG_WINDOW_MS);
    if (!tagValid) {
      unknownCount++;
      showAlert("UNKNOWN PERSON", "Not counted!", 3000);
      alarmBeep();
      logEvent("UNKNOWN", "Entry without valid RFID", true);
    } else {
      Person& p = people[pendingIdx];
      if (p.inside) {
        showAlert("Already inside", p.name, ALERT_MS);
        snprintf(m, sizeof(m), "%s tried to enter twice", p.name);
        logEvent("DUPLICATE", m, false);
      } else {
        p.inside = true;
        totalCount++;

        // Only mark teacher present when the teacher's card was used to enter
        if (p.isTeacher) {
          teacherPresent = true;
        } else {
          // Record student into activeStudents stack with entrance timestamp
          if (activeStudentCount < 4) {
            char ts[9]; stamp(ts);
            activeStudents[activeStudentCount].personIdx = pendingIdx;
            strncpy(activeStudents[activeStudentCount].enterTime, ts, sizeof(activeStudents[activeStudentCount].enterTime));
            activeStudents[activeStudentCount].enterTime[sizeof(activeStudents[activeStudentCount].enterTime) - 1] = '\0';
            activeStudentCount++;
          }
        }
        studentCount = activeStudentCount;

        showAlert(p.isTeacher ? "Teacher ENTERED" : "Student ENTERED", p.name, ALERT_MS);
        snprintf(m, sizeof(m), "%s entered, total=%d", p.name, totalCount);
        logEvent("ENTRY", m, false);
      }
    }
  } else {
    // EXITING (IR2 -> IR1): Contactless free exit (no RFID tap required)
    if (totalCount > 0) {
      totalCount--;
    }

    // Rule: if total count is 0 then teacher is absent, otherwise teacher is present
    if (totalCount == 0) {
      teacherPresent = false;
      studentCount   = 0;
      unknownCount   = 0; // Reset unknown counter when room is fully vacated
      activeStudentCount = 0;
      for (uint8_t i = 0; i < 4; i++) {
        people[i].inside = false; // Reset inside flags for clean re-entry
      }
      showAlert("Classroom Empty", "All power OFF", ALERT_MS);
      logEvent("EXIT", "Last person left, room empty", false);
      beep(80, 1000);
    } else {
      // While leaving the class, decrease students from descending order (LIFO - latest entered leaves first)
      if (activeStudentCount > 0) {
        activeStudentCount--;
        uint8_t leavingIdx = activeStudents[activeStudentCount].personIdx;
        people[leavingIdx].inside = false; // Mark student as outside so they can re-enter cleanly!
      }
      studentCount = activeStudentCount;

      snprintf(m, sizeof(m), "Person left, total=%d", totalCount);
      showAlert("Person LEFT", m, ALERT_MS);
      logEvent("EXIT", m, false);
      beep(50, 1200);
    }
  }

  pendingIdx = -1;
  applyAutomation();
  dirtyLive = true;
  dirtyHistory = true;
  firebaseSyncLive();
  lastLiveSync = millis();
}


// ---------------- 2-SENSOR DIRECTION TRACKING ----------------
void updateIR() {
  bool b1 = (digitalRead(PIN_IR1) == LOW);
  bool b2 = (digitalRead(PIN_IR2) == LOW);
  bool r1 = b1 && !prevB1;
  bool r2 = b2 && !prevB2;
  prevB1 = b1;
  prevB2 = b2;

  switch (irState) {
    case IR_IDLE:
      if (r1 && !r2)      { irState = IR_A_FIRST; irT = millis(); }
      else if (r2 && !r1) { irState = IR_B_FIRST; irT = millis(); }
      break;

    case IR_A_FIRST:      // IR1 -> IR2 (ENTRY)
      if (r2) {
        handleCrossing(true);
        irState = IR_WAIT_CLEAR;
      } else if (millis() - irT > IR_TIMEOUT_MS) {
        irState = IR_WAIT_CLEAR;
      }
      break;

    case IR_B_FIRST:      // IR2 -> IR1 (EXIT)
      if (r1) {
        handleCrossing(false);
        irState = IR_WAIT_CLEAR;
      } else if (millis() - irT > IR_TIMEOUT_MS) {
        irState = IR_WAIT_CLEAR;
      }
      break;

    case IR_WAIT_CLEAR:
      if (!b1 && !b2) {
        irState = IR_IDLE;
      }
      break;
  }
}

// Read the 5 on-screen Card Buttons with debounce
void updateCards() {
  unsigned long nowMs = millis();
  for (uint8_t i = 0; i < 5; i++) {
    bool now = (digitalRead(CARD_PINS[i]) == HIGH); // HIGH = not pressed (active-LOW buttons)
    // Detect falling edge (button just pressed) with debounce guard
    if (prevCard[i] && !now) {
      if (nowMs - lastDebounce[i] >= DEBOUNCE_MS) {
        lastDebounce[i] = nowMs;
        onCard(i);
      }
    }
    prevCard[i] = now;
  }
}

// ---------------- LCD SCREEN RENDERER ----------------
void renderLCD() {
  char l1[20], l2[20];
  if (millis() < alertUntil) {
    lcdLine(0, alertL1);
    lcdLine(1, alertL2);
    return;
  }

  if (millis() - lastPage >= PAGE_MS) {
    lastPage = millis();
    page = (page + 1) % 3;
  }

  if (page == 0) {
    snprintf(l1, sizeof(l1), "Tchr: %-10s", teacherPresent ? "Present" : "Absent");
    snprintf(l2, sizeof(l2), "Total:%-2d Stud:%-2d", totalCount, studentCount);
  } else if (page == 1) {
    snprintf(l1, sizeof(l1), "Temp: %d.%d C", tempX10 / 10, abs(tempX10 % 10));
    snprintf(l2, sizeof(l2), "Unknown alerts:%d", unknownCount);
  } else {
    snprintf(l1, sizeof(l1), "Fan:%-3s Lght:%-3s", fanOn ? "ON" : "OFF", lightOn ? "ON" : "OFF");
    snprintf(l2, sizeof(l2), "AC:%-3s Proj:%-3s",  acOn ? "ON" : "OFF",  projOn ? "ON" : "OFF");
  }

  lcdLine(0, l1);
  lcdLine(1, l2);
}

// ---------------- SERIAL COMMAND INTERFACE ----------------
void printStatus() {
  Serial.println(F("------ CLASSROOM STATUS ------"));
  Serial.print(F("Teacher  : ")); Serial.println(teacherPresent ? F("PRESENT") : F("ABSENT"));
  Serial.print(F("Total    : ")); Serial.println(totalCount);
  Serial.print(F("Students : ")); Serial.println(studentCount);
  Serial.print(F("Unknown  : ")); Serial.println(unknownCount);
  Serial.print(F("Temp     : ")); Serial.print(tempX10 / 10); Serial.print('.'); Serial.println(abs(tempX10 % 10));
  Serial.print(F("Fan="));    Serial.print(fanOn);
  Serial.print(F(" Light="));  Serial.print(lightOn);
  Serial.print(F(" AC="));     Serial.print(acOn);
  Serial.print(F(" Proj="));   Serial.println(projOn);
  Serial.println(F("------------------------------"));
}

void runCommand(const char* c) {
  if      (!strcmp(c, "T"))      onCard(0);
  else if (!strcmp(c, "S1"))     onCard(1);
  else if (!strcmp(c, "S2"))     onCard(2);
  else if (!strcmp(c, "S3"))     onCard(3);
  else if (!strcmp(c, "U"))      onCard(4);
  else if (!strcmp(c, "IN"))     handleCrossing(true);
  else if (!strcmp(c, "OUT"))    handleCrossing(false);
  else if (!strcmp(c, "STATUS")) printStatus();
  else if (!strcmp(c, "SYNC"))   { Serial.println(F("[COMMAND] Forcing Firebase Sync...")); firebaseSyncLive(); }
  else Serial.println(F("Commands: T S1 S2 S3 U | IN OUT | STATUS | SYNC"));
}

void handleSerial() {
  while (Serial.available()) {
    char ch = Serial.read();
    lastRx = millis();
    if (ch == '\n' || ch == '\r') {
      if (cmdLen) { cmdBuf[cmdLen] = 0; runCommand(cmdBuf); cmdLen = 0; }
    } else if (cmdLen < sizeof(cmdBuf) - 1) {
      cmdBuf[cmdLen++] = toupper(ch);
    }
  }
  // If serial monitor doesn't send newline, run after short pause
  if (cmdLen && (millis() - lastRx > 150)) {
    cmdBuf[cmdLen] = 0;
    runCommand(cmdBuf);
    cmdLen = 0;
  }
}

// ---------------- SETUP & LOOP ----------------
void setup() {
  Serial.begin(115200);

  // Initialize Relays as OUTPUT and default to OFF (LOW)
  const uint8_t outs[4] = { PIN_FAN, PIN_LIGHT, PIN_AC, PIN_PROJ };
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(outs[i], OUTPUT);
    digitalWrite(outs[i], LOW);
  }

  pinMode(PIN_BUZ, OUTPUT);
  pinMode(PIN_IR1, INPUT_PULLUP);
  pinMode(PIN_IR2, INPUT_PULLUP);

  // Card Pushbuttons with internal pullup
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(CARD_PINS[i], INPUT_PULLUP);
  }
  pinMode(CARD_PINS[4], INPUT); // Pin 34 uses external pullup resistor

  // Initialize LCD
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcdLine(0, "Smart Classroom");
  lcdLine(1, "Starting...");
  beep(120, 1800);
  delay(1000);

  // Initialize SPI & MFRC522 RFID
  SPI.begin(18, 19, 23, PIN_SS);
  rfid.PCD_Init();

  // Initialize DHT22
  dht.begin();

  // Connect to Wi-Fi (Wokwi-GUEST for simulation, or configured AP for hardware)
  Serial.print(F("[WIFI] Connecting to "));
  Serial.print(WIFI_SSID);
  Serial.print(F("..."));
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 12) {
    delay(300);
    Serial.print('.');
    retries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F(" Connected! IP: "));
    Serial.println(WiFi.localIP());
    // Configure NTP time for Bangladesh (UTC+6 = 21600 seconds)
    configTime(21600, 0, "pool.ntp.org", "time.google.com");
    firebaseSyncLive();
  } else {
    Serial.println(F(" Continuing in offline simulation mode."));
  }

  readTemperature();
  applyAutomation();
  Serial.println(F("Smart Classroom ready. Commands: T S1 S2 S3 U | IN OUT | STATUS"));
  printStatus();
}

void loop() {
  handleSerial();
  checkMFRC522();
  updateCards();
  updateIR();

  // Auto-reconnect if Wi-Fi momentarily drops
  static unsigned long lastWifiCheck = 0;
  if (millis() - lastWifiCheck >= 10000UL) {
    lastWifiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println(F("[WIFI] Reconnecting..."));
      WiFi.reconnect();
    }
  }

  if (millis() - lastTemp >= TEMP_MS) {
    lastTemp = millis();
    readTemperature();
    applyAutomation();
  }

  if (millis() - lastRender >= 400) {
    lastRender = millis();
    renderLCD();
  }

  // Fast Live Synchronization (sub-second responsiveness for card taps & door crossings)
  if ((dirtyLive && (millis() - lastLiveSync >= 500)) || (millis() - lastLiveSync >= 30000UL)) {
    firebaseSyncLive();
    dirtyLive = false;
    lastLiveSync = millis();
  }

  // Periodic history snapshots (only on meaningful classroom events, or 60s background heartbeat)
  if ((dirtyHistory && (millis() - lastHistorySync >= 1500)) || (millis() - lastHistorySync >= 60000UL)) {
    firebasePushHistory();
    dirtyHistory = false;
    lastHistorySync = millis();
  }
}

