# Phase 2: Firebase Realtime Database Integration Guide

**Smart Classroom Management and Automation System**  
**Team 06 — CSE 315 / 316 (University of Asia Pacific)**

---

## 🌐 1. Architecture Overview

```
 ┌─────────────────────────────────────────────────────────┐
 │                   ESP32 Microcontroller                 │
 │  (Wokwi Virtual Wi-Fi / Physical Wi-Fi Hotspot)         │
 └────────────────────────────┬────────────────────────────┘
                              │ HTTPS REST API
                              ▼
 ┌─────────────────────────────────────────────────────────┐
 │             Firebase Realtime Database (Google Cloud)   │
 │                                                         │
 │  ├── /classroom/live    (Real-time LCD state & loads)   │
 │  ├── /classroom/alerts  (Unauthorized intruder logs)   │
 │  └── /classroom/history (Timestamped telemetry history) │
 └────────────────────────────┬────────────────────────────┘
                              │ Real-Time WebSocket Listener
                              ▼
 ┌─────────────────────────────────────────────────────────┐
 │              Android Mobile Application (Phase 3)       │
 │   - Teacher/Admin Login Authentication                  │
 │   - Live Classroom Monitoring Dashboard                 │
 │   - Timestamped Unknown Intruder Alert Feed             │
 └─────────────────────────────────────────────────────────┘
```

---

## 🚀 2. Step-by-Step: 3-Minute Free Firebase Setup

Follow these exact steps to create your free Google Firebase backend:

### Step 1: Create a Firebase Project
1. Open your browser and go to [Firebase Console](https://console.firebase.google.com/).
2. Log in with any Google account and click **"Add project"** (or **"Create a project"**).
3. Project Name: `smart-classroom-team06` (or any name you prefer).
4. Disable Google Analytics (optional, not needed for this project) and click **Create Project**.

### Step 2: Create the Realtime Database
1. In the left navigation menu, expand **Build** and click **Realtime Database**.
2. Click **Create Database**.
3. **Database Location**: Choose `United States (us-central1)` or `Singapore (asia-southeast1)`.
4. **Security Rules**: Select **Start in test mode** (allows read & write during development for 30 days):
   ```json
   {
     "rules": {
       ".read": true,
       ".write": true
     }
   }
   ```
5. Click **Enable**.

### Step 3: Copy Your Database URL
At the top of the Realtime Database page, you will see your database URL. It looks like:
```text
https://smart-classroom-team06-default-rtdb.firebaseio.com
```
*(Or with a region prefix, like `https://smart-classroom-team06-default-rtdb.asia-southeast1.firebasedatabase.app`)*.

---

## ⚡ 3. Linking the ESP32 (Wokwi & Physical)

Open [`sketch.ino`](file:///d:/Study/3.2/CSE-315/Project/simulation/wokwi_esp32_simulation/sketch.ino) and update lines 18–24:

```cpp
// ---------------- FIREBASE & WI-FI CONFIGURATION ----------------
// For Wokwi simulation: SSID = "Wokwi-GUEST", Password = ""
// For physical ESP32: enter your mobile hotspot / home Wi-Fi credentials
const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASS     = "";

// Paste your Firebase Realtime Database URL (MUST start with https:// and have NO trailing slash)
const char* FIREBASE_URL  = "https://YOUR-PROJECT-ID-default-rtdb.firebaseio.com";
const char* FIREBASE_AUTH = ""; // Leave empty for Test Mode
```

### How Wi-Fi Works:
* **In Wokwi Simulation**: Wokwi automatically connects to its simulated virtual router `Wokwi-GUEST`. It has direct access to the live Internet via Google's backend, so your simulated ESP32 talks to real Firebase servers in real time!
* **In Physical Prototype**: Change `WIFI_SSID` to your smartphone hotspot or lab Wi-Fi name, and `WIFI_PASS` to the hotspot password.

---

## 🗄️ 4. Database Schema Structure

The ESP32 communicates with 3 dedicated database paths:

### 1. `/classroom/live` (Live State Mirror)
Updated automatically whenever occupancy, temperature, or appliances change state:
```json
{
  "teacherPresent": true,
  "teacherName": "Sayma Ma'am",
  "studentCount": 2,
  "temperature": 27.5,
  "fan": true,
  "light": true,
  "ac": false,
  "projector": true,
  "lastUpdated": "03:15:20"
}
```

### 2. `/classroom/alerts` (Intrusion Detection Logs)
Appended automatically every time an unregistered RFID card is tapped or a person enters without scanning:
```json
{
  "-ODx82mN28hds7": {
    "type": "UNKNOWN_PERSON",
    "message": "Entry without valid RFID",
    "time": "03:18:42",
    "students": 2,
    "teacherPresent": true
  }
}
```

### 3. `/classroom/history` (Periodic Cloud Records)
Logged every 30 seconds for analytics, attendance records, and temperature trends:
```json
{
  "-ODx894hKns82b": {
    "time": "03:19:00",
    "teacher": "PRESENT",
    "students": 2,
    "temp": 27.5,
    "fan": 1,
    "light": 1,
    "ac": 0,
    "proj": 1
  }
}
```

---

## 🧪 5. Testing the Cloud Sync in Wokwi

1. Open your project in [Wokwi](https://wokwi.com).
2. Open your [Firebase Console](https://console.firebase.google.com/) Realtime Database tab side-by-side with Wokwi.
3. Click **Start Simulation** in Wokwi.
   * Serial Monitor will print:
     ```text
     [WIFI] Connecting to Wokwi-GUEST... Connected! IP: 10.0.x.x
     [FIREBASE PUT] /classroom/live.json -> HTTP 200
     ```
4. **Test 1 — Student Entry**:
   * Click **S1 (Hasanul)**, then click **IR1** $\rightarrow$ **IR2**.
   * Look at Firebase: `studentCount` turns into `1`, `light` becomes `true` **in under 0.5 seconds**!
5. **Test 2 — Teacher Enters and Leaves**:
   * Tap **T (Sayma Ma'am)** $\rightarrow$ **IR1** $\rightarrow$ **IR2**.
   * In Firebase: `teacherPresent` = `true`, `projector` = `true`.
   * Tap **T (Sayma Ma'am)** $\rightarrow$ **IR2** $\rightarrow$ **IR1** (Teacher exits).
   * In Firebase: `teacherPresent` = `false`, `projector` = `false`, but **`light` remains `true`**!
6. **Test 3 — Unknown Intruder Alert**:
   * Click **IR1** then **IR2** with no card tap (or click the grey **Unknown Card** button).
   * Buzzer sounds, and an instant alert is pushed under `/classroom/alerts` with the exact timestamp!

---

## 📱 6. Ready for Phase 3: Android Mobile App

Now that Firebase receives real-time telemetry from the ESP32:
* Authorized users (Teachers / Admins) can log into the Android app.
* The app attaches a Firebase `ValueEventListener` to `/classroom/live` for instantaneous dashboard updates without lag.
* An alerts screen queries `/classroom/alerts` to notify faculty of unauthorized entries with date and time.
