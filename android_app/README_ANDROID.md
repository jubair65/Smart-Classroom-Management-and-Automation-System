# Smart Classroom Mobile Application (Native Android)

**Department of Computer Science and Engineering**  
**University of Asia Pacific (UAP)**  
*Course Code: CSE 316 (Microprocessors and Microcontrollers Lab)*  
*Team Number: 06 — Section B1*

---

## 📱 Project Overview

This native Android application satisfies **Section 4 ("Mobile App Integration")** of the official project proposal:
* **Authorized Access**: Secure login interface restricting monitoring access to verified faculty and system administrators.
* **Real-Time Telemetry Dashboard**: Direct, event-driven listening to Firebase Realtime Database (`/classroom/live`) for teacher presence, student count, room temperature, and appliance states.
* **Intruder Alert Feed**: Instant push alerts with timestamps whenever an unauthorized individual enters or an unregistered RFID card is tapped.
* **Historical Telemetry Log**: Review past classroom energy, occupancy, and temperature records over time.

---

## 🛠️ Tech Stack & Architecture

* **Language**: Kotlin 1.9+
* **Min SDK**: API 24 (Android 7.0+) · **Target SDK**: API 34 (Android 14)
* **Architecture**: Clean MVVM with Material Design 3 and AndroidX ViewBinding
* **Cloud Backend**: Google Firebase Realtime Database
* **Real-Time Engine**: WebSocket-based `ValueEventListener` (zero lag, instant UI updates without manual polling)

---

## 📂 Project Structure

```text
android_app/
├── app/
│   ├── src/main/
│   │   ├── AndroidManifest.xml
│   │   ├── java/com/uap/cse316/smartclassroom/
│   │   │   ├── data/model/
│   │   │   │   ├── ClassroomLive.kt      <── Live telemetry mapping
│   │   │   │   ├── AlertItem.kt          <── Intrusion alert schema
│   │   │   │   └── HistoryItem.kt        <── 30-second snapshot schema
│   │   │   ├── ui/
│   │   │   │   ├── login/
│   │   │   │   │   └── LoginActivity.kt  <── Teacher / Admin authentication
│   │   │   │   ├── main/
│   │   │   │   │   └── MainActivity.kt   <── Bottom nav & fragment container
│   │   │   │   ├── dashboard/
│   │   │   │   │   └── DashboardFragment.kt <── Live classroom monitor
│   │   │   │   ├── alerts/
│   │   │   │   │   ├── AlertsFragment.kt <── Security intrusion feed
│   │   │   │   │   └── AlertAdapter.kt
│   │   │   │   └── history/
│   │   │   │       ├── HistoryFragment.kt<── Telemetry log timeline
│   │   │   │       └── HistoryAdapter.kt
│   │   │   └── utils/
│   │   │       └── FirebaseManager.kt    <── Database URL & singleton refs
│   │   └── res/
│   │       ├── layout/                   <── Modern Material 3 card layouts
│   │       ├── menu/                     <── Bottom nav & toolbar menus
│   │       └── values/                   <── Color tokens, styles, strings
│   ├── build.gradle.kts
│   └── google-services.json
├── build.gradle.kts
└── settings.gradle.kts
```

---

## 🚀 How to Open and Run in Android Studio

### Step 1: Open the Project
1. Launch **Android Studio**.
2. Click **Open** (or `File > Open...`).
3. Browse to:
   ```text
   d:\Study\3.2\CSE-315\Project\simulation\android_app
   ```
4. Click **OK**. Android Studio will automatically download Gradle dependencies and perform project synchronization.

### Step 2: Run the App
1. Connect an Android phone via USB (with **USB Debugging** enabled), OR select an Android Virtual Device (AVD Emulator).
2. Click the green **Run** button (`Shift + F10`).

---

## 🔑 Authorized Credentials (for Lab Evaluation)

The app enforces authorized access as specified in the report:

| Role | Email | Password | Access Rights |
| :--- | :--- | :--- | :--- |
| **Faculty (Teacher)** | `teacher@uap.edu` | `123456` | Full monitoring & alert feed |
| **Lab Administrator** | `admin@uap.edu` | `123456` | Full system oversight & history |

*(Pre-filled demo credentials are provided directly on the login screen for rapid presentation).*

---

## 🧪 Live Demonstration with Wokwi Simulation

1. Start your **Wokwi ESP32 simulation** in your browser.
2. Open the **Smart Classroom Android App** on your phone or emulator and log in.
3. Observe real-time synchronization:
   * **Tap Teacher Card (`T`) $\rightarrow$ Door Crossing (`IN`)**:
     * In Wokwi: Projector turns ON, Light turns ON.
     * In Android App: Teacher badge switches to **PRESENT (Green)**, Projector card glows **ON (Purple)** in $<0.5\text{s}$!
   * **Simulate Unknown Intruder (`U` or cross door without card)**:
     * In Wokwi: Buzzer sounds 3 beeps, LCD shows `UNKNOWN PERSON / Not counted!`.
     * In Android App: Switch to the **Intruder Alerts** tab $\rightarrow$ A red alert card pops up instantly showing the exact timestamp and context!
   * **Teacher Exits (`T` then `OUT`) while student remains (`S1`)**:
     * In Wokwi: Projector turns OFF, but Light and Fan remain ON.
     * In Android App: Teacher badge switches to **ABSENT (Red)**, Projector turns **OFF**, while **Student Count = 1** and Light remains **ON (Yellow)**!
