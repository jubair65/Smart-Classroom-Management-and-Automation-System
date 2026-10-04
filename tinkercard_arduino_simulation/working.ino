// ==============================================================================
// Smart Classroom Management and Automation System (Tinkercad Circuits)
// Team 06 - CSE 315 / 316
// ==============================================================================

// Include Wire for I2C communication (mandatory in Tinkercad for I2C LCD)
#include <Wire.h>
//#include <Adafruit_LiquidCrystal.h>

// If your Tinkercad simulation uses the LiquidCrystal_I2C library instead,
// comment out the Adafruit lines above and uncomment the two lines below:
// #include <LiquidCrystal_I2C.h>
// LiquidCrystal_I2C lcd(0x27, 16, 2); // or address 0x20

// Adafruit I2C LCD (offset 0 = default 0x20 in Tinkercad)
//Adafruit_LiquidCrystal lcd(0);

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); // or 0x20


// ---------------- PIN DEFINITIONS ----------------
const uint8_t PIN_IR1       = 2;    // Outside IR sensor (button)
const uint8_t PIN_IR2       = 3;    // Inside IR sensor (button)
const uint8_t CARD_PINS[5]  = { 4, 5, 6, 7, 8 };  // Teacher, Student 1, Student 2, Student 3, Unknown
const uint8_t PIN_BUZ       = 9;    // Buzzer / Speaker
const uint8_t PIN_FAN       = 10;   // Fan relay driver
const uint8_t PIN_LIGHT     = 11;   // Light LED / relay
const uint8_t PIN_AC        = 12;   // AC LED / relay
const uint8_t PIN_PROJ      = 13;   // Projector LED / relay
const uint8_t PIN_TEMP      = A0;   // TMP36 Temperature sensor
// LCD I2C Pins: SDA = A4, SCL = A5

// ---------------- SETTINGS & THRESHOLDS ----------------
const int FAN_ON_X10        = 270;  // 27.0 deg C (temp * 10)
const int FAN_OFF_X10       = 255;  // 25.5 deg C
const int AC_ON_X10         = 300;  // 30.0 deg C
const int AC_OFF_X10        = 285;  // 28.5 deg C

const unsigned long TAG_WINDOW_MS = 10000UL; // 10 seconds to cross door after card tap
const unsigned long IR_TIMEOUT_MS = 3000UL;  // 3 seconds between sensor 1 and sensor 2
const unsigned long ALERT_MS      = 2500UL;  // Alert display duration
const unsigned long PAGE_MS       = 3000UL;  // LCD page rotate interval
const unsigned long TEMP_MS       = 1000UL;  // Temperature check interval
const unsigned long RECORD_MS     = 30000UL; // Cloud log interval

// ---------------- USER REGISTRY ----------------
struct Person {
  char name[8];
  bool isTeacher;
  bool inside;
};

Person people[4] = {
  { "Sayma",   true,  false },
  { "Hasanul", false, false },
  { "Jubair",  false, false },
  { "Maria",   false, false }
};

// ---------------- SYSTEM STATE ----------------
int  tempX10        = 0;
int  studentCount   = 0;
int  unknownCount   = 0;
bool teacherPresent = false;
bool fanOn   = false;
bool lightOn = false;
bool acOn    = false;
bool projOn  = false;

int8_t pendingIdx         = -1;
unsigned long pendingTime = 0;

enum IRState { IR_IDLE, IR_A_FIRST, IR_B_FIRST, IR_WAIT_CLEAR };
IRState irState = IR_IDLE;
unsigned long irT = 0;
bool prevB1 = false;
bool prevB2 = false;
bool prevCard[5] = { true, true, true, true, true };

char alertL1[17] = "";
char alertL2[17] = "";
unsigned long alertUntil = 0;
unsigned long lastPage   = 0;
unsigned long lastRender = 0;
unsigned long lastTemp   = 0;
unsigned long lastRecord = 0;
uint8_t page = 0;
bool dirty = true;

char cmdBuf[12];
uint8_t cmdLen = 0;
unsigned long lastRx = 0;

// ---------------- HELPER FUNCTIONS ----------------
void stamp(char* out) {
  unsigned long s = millis() / 1000UL;
  sprintf(out, "%02lu:%02lu:%02lu", (s / 3600UL) % 100UL, (s / 60UL) % 60UL, s % 60UL);
}

void lcdLine(uint8_t row, const char* s) {
  char b[17];
  snprintf(b, sizeof(b), "%-16s", s);
  lcd.setCursor(0, row);
  lcd.print(b);
}

void beep(unsigned int ms, unsigned int freq) {
  tone(PIN_BUZ, freq, ms);
  delay(ms);
}

void alarmBeep() {
  for (uint8_t i = 0; i < 3; i++) {
    beep(150, 2000);
    delay(100);
  }
}

void showAlert(const char* l1, const char* l2, unsigned long ms) {
  strncpy(alertL1, l1, 16);
  alertL1[16] = '\0';
  strncpy(alertL2, l2, 16);
  alertL2[16] = '\0';
  alertUntil = millis() + ms;
  lastRender = 0;
}

void logEvent(const char* type, const char* detail, bool unknown) {
  char ts[9];
  stamp(ts);
  Serial.print('[');
  Serial.print(ts);
  Serial.print(F("] "));
  Serial.print(type);
  Serial.print(F(" : "));
  Serial.println(detail);
  if (unknown) {
    Serial.print(F("ALERT> Unknown person detected at "));
    Serial.println(ts);
  }
}

void cloudRecord() {
  char ts[9];
  stamp(ts);
  char b[120];
  snprintf(b, sizeof(b),
    "{\"time\":\"%s\",\"teacher\":\"%s\",\"students\":%d,\"temp\":%d.%d,\"fan\":%d,\"light\":%d,\"ac\":%d,\"proj\":%d}",
    ts,
    teacherPresent ? "PRESENT" : "ABSENT",
    studentCount,
    tempX10 / 10,
    abs(tempX10 % 10),
    fanOn ? 1 : 0,
    lightOn ? 1 : 0,
    acOn ? 1 : 0,
    projOn ? 1 : 0
  );
  Serial.print(F("CLOUD> "));
  Serial.println(b);
  dirty = false;
  lastRecord = millis();
}

// ---------------- TEMPERATURE READING (TMP36) ----------------
void readTemperature() {
  long sum = 0;
  for (uint8_t i = 0; i < 8; i++) {
    sum += analogRead(PIN_TEMP);
  }
  long raw = sum / 8;
  long mv = raw * 5000L / 1024L;          // Millivolts
  tempX10 = (int)(mv - 500L);             // TMP36: (mV - 500) / 10 = deg C -> x10
}

// ---------------- AUTOMATION LOGIC ----------------
void applyAutomation() {
  bool occupied = teacherPresent || (studentCount > 0);
  bool nLight = occupied;
  bool nProj  = teacherPresent;
  bool nFan   = fanOn;
  bool nAC    = acOn;

  if (!occupied) {
    nFan = false;
    nAC  = false;
  } else {
    if (tempX10 >= FAN_ON_X10)       nFan = true;
    else if (tempX10 <= FAN_OFF_X10) nFan = false;

    if (tempX10 >= AC_ON_X10)        nAC = true;
    else if (tempX10 <= AC_OFF_X10)  nAC = false;
  }

  if (nLight != lightOn || nProj != projOn || nFan != fanOn || nAC != acOn) {
    dirty = true;
  }
  lightOn = nLight;
  projOn  = nProj;
  fanOn   = nFan;
  acOn    = nAC;

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
    return;
  }

  pendingIdx = idx;
  pendingTime = millis();
  showAlert(people[idx].isTeacher ? "Teacher card OK" : "Student card OK", people[idx].name, 2000);
  beep(80, 1500);
  snprintf(m, sizeof(m), "%s tapped card", people[idx].name);
  logEvent("CARD", m, false);
}

// ---------------- DOOR PASSAGE DETECTION ----------------
void handleCrossing(bool entering) {
  bool tagValid = (pendingIdx >= 0) && ((millis() - pendingTime) <= TAG_WINDOW_MS);
  char m[40];

  if (!tagValid) {
    if (entering) {
      unknownCount++;
      showAlert("UNKNOWN PERSON", "Not counted!", 3000);
      alarmBeep();
      logEvent("UNKNOWN", "Entry without valid RFID", true);
    } else {
      showAlert("Exit: no card", "Tap card to exit", ALERT_MS);
      logEvent("EXIT-NOCARD", "Exit without card tap", false);
    }
  } else {
    Person& p = people[pendingIdx];
    if (entering) {
      if (p.inside) {
        showAlert("Already inside", p.name, ALERT_MS);
        snprintf(m, sizeof(m), "%s tried to enter twice", p.name);
        logEvent("DUPLICATE", m, false);
      } else {
        p.inside = true;
        if (p.isTeacher) teacherPresent = true;
        else studentCount++;

        showAlert(p.isTeacher ? "Teacher ENTERED" : "Student ENTERED", p.name, ALERT_MS);
        snprintf(m, sizeof(m), "%s entered, students=%d", p.name, studentCount);
        logEvent("ENTRY", m, false);
      }
    } else {
      if (!p.inside) {
        showAlert("Not inside", p.name, ALERT_MS);
        snprintf(m, sizeof(m), "%s exit but not inside", p.name);
        logEvent("DUPLICATE", m, false);
      } else {
        p.inside = false;
        if (p.isTeacher) teacherPresent = false;
        else if (studentCount > 0) studentCount--;

        showAlert(p.isTeacher ? "Teacher LEFT" : "Student LEFT", p.name, ALERT_MS);
        snprintf(m, sizeof(m), "%s left, students=%d", p.name, studentCount);
        logEvent("EXIT", m, false);
      }
    }
  }

  pendingIdx = -1;
  applyAutomation();
  dirty = true;
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

void updateCards() {
  for (uint8_t i = 0; i < 5; i++) {
    bool now = digitalRead(CARD_PINS[i]);
    if (prevCard[i] && !now) onCard(i);
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
    page = (page + 1) % 3;
    lastPage = millis();
  }

  if (page == 0) {
    snprintf(l1, sizeof(l1), "Teacher:%s", teacherPresent ? "PRESENT" : "ABSENT");
    snprintf(l2, sizeof(l2), "Students: %d", studentCount);
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
  Serial.print(F("Students : ")); Serial.println(studentCount);
  Serial.print(F("Unknown  : ")); Serial.println(unknownCount);
  Serial.print(F("Temp     : ")); Serial.print(tempX10 / 10); Serial.print('.'); Serial.println(abs(tempX10 % 10));
  Serial.print(F("Fan="));   Serial.print(fanOn);
  Serial.print(F(" Light=")); Serial.print(lightOn);
  Serial.print(F(" AC="));    Serial.print(acOn);
  Serial.print(F(" Proj="));  Serial.println(projOn);
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
  else Serial.println(F("Commands: T S1 S2 S3 U | IN OUT | STATUS"));
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
  // Tinkercad's serial box may not send a newline: run the command after a short pause
  if (cmdLen && (millis() - lastRx > 150)) {
    cmdBuf[cmdLen] = 0;
    runCommand(cmdBuf);
    cmdLen = 0;
  }
}

// ---------------- SETUP & LOOP ----------------
void setup() {
  Serial.begin(9600);

  const uint8_t outs[4] = { PIN_FAN, PIN_LIGHT, PIN_AC, PIN_PROJ };
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(outs[i], OUTPUT);
    digitalWrite(outs[i], LOW);
  }
  pinMode(PIN_BUZ, OUTPUT);
  pinMode(PIN_IR1, INPUT_PULLUP);
  pinMode(PIN_IR2, INPUT_PULLUP);
  for (uint8_t i = 0; i < 5; i++) {
    pinMode(CARD_PINS[i], INPUT_PULLUP);
  }

  // Adafruit LiquidCrystal initialization
  //lcd.begin(16, 2);
  //lcd.setBacklight(1);

  // If using LiquidCrystal_I2C instead, use:
  lcd.init();
   lcd.backlight();

  lcdLine(0, "Smart Classroom");
  lcdLine(1, "Starting...");
  beep(120, 1800);
  delay(1200);

  readTemperature();
  applyAutomation();
  Serial.println(F("Smart Classroom ready. Commands: T S1 S2 S3 U | IN OUT | STATUS"));
  printStatus();
}

void loop() {
  handleSerial();
  updateCards();
  updateIR();

  if (millis() - lastTemp >= TEMP_MS) {
    lastTemp = millis();
    readTemperature();
    applyAutomation();
  }

  if (millis() - lastRender >= 400) {
    lastRender = millis();
    renderLCD();
  }

  if ((dirty && (millis() - lastRecord > 500)) || (millis() - lastRecord > RECORD_MS)) {
    cloudRecord();
  }
}
