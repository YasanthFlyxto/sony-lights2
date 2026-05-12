# 🔆 SonyLights2 — Setup Guide
**Simple Wireless RGB Lamp Control | ESP32 + WS2812B**

---

## 📦 What You Need

| Item | Qty |
|---|---|
| ESP32 Dev Board | 12 |
| WS2812B LED Strip (5V) | 12 |
| 10000mAh USB Powerbank | 12 |
| USB-A to Micro-USB cable | 12 (to power ESP32 from bank) |
| 330Ω resistor | 12 (optional but recommended on data line) |
| WiFi Router | 1 |
| Laptop / Tablet | 1 (to open dashboard) |

---

## 🔌 Wiring Each Lamp

```
WS2812B Strip          ESP32
──────────────        ─────────
  5V    ──────────►  VIN  (or 5V pin)
  GND   ──────────►  GND
  DIN   ──[330Ω]──►  GPIO 5
```

> ⚠️ **Power rule:** Power the LED strip DIRECTLY from the USB powerbank 5V output (or a USB splitter cable).  
> Do NOT power the strip through the ESP32's 3.3V pin — it cannot supply enough current.  
> The ESP32 can be powered separately by another USB cable from the same powerbank.

---

## 💻 Software Setup (one time)

### Step 1 — Install Arduino IDE
Download from: https://www.arduino.cc/en/software  
(Version 2.x recommended)

### Step 2 — Add ESP32 Board
1. Open Arduino IDE → **File → Preferences**
2. In "Additional Board URLs", paste:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools → Board → Boards Manager**
4. Search **ESP32** → Install "esp32 by Espressif Systems"

### Step 3 — Install FastLED Library
1. **Tools → Manage Libraries**
2. Search **FastLED** → Install (by Daniel Garcia)

---

## ⚡ Flash Each ESP32

1. Open `firmware/lamp_firmware/lamp_firmware.ino` in Arduino IDE

2. **Change these 3 lines at the top** before each flash:
   ```cpp
   const char* WIFI_SSID = "YourWiFi";      // Your WiFi name
   const char* WIFI_PASS = "YourPassword";  // Your WiFi password
   const int   LAMP_ID   = 1;              // Change: 1, 2, 3 ... 12
   ```

3. Select Board: **Tools → Board → ESP32 Dev Module**

4. Select the correct **COM Port** (appears when you plug in ESP32)

5. Click **Upload (→)**

6. Open **Serial Monitor** (115200 baud) — wait for this line:
   ```
   ✅ Lamp 1 online at: http://192.168.1.XXX
   ```
   **Write down that IP address!** You need it for the dashboard.

7. The strip will flash:
   - 🟠 Orange = connecting to WiFi
   - 🟢 Green = connected successfully
   - 🔴 Red = failed (check SSID/Password)

8. Repeat steps 2–7 for all 12 ESP32s, changing `LAMP_ID` each time.

---

## 🎛️ Using the Dashboard

1. Make sure your laptop/tablet is on the **same WiFi** as the lamps

2. Open `dashboard/index.html` in **Google Chrome**  
   *(just double-click the file)*

3. Click **⚙ Settings**:
   - Enter the **Base IP** (e.g. `192.168.1`) then click **Auto-fill**  
     *This fills Lamp 1 = .101, Lamp 2 = .102 ... Lamp 12 = .112*
   - Or manually enter each IP you noted from Serial Monitor
   - Click **Save & Close**

4. The dashboard polls every 4 seconds — online lamps show **green dots**

5. **Master Control** (top):
   - Pick color → choose effect → set brightness → click ⚡ **SYNC ALL**

6. **Individual cards**: adjust brightness or toggle power per lamp

---

## 📶 LED Status Codes During Boot

| Color | Meaning |
|---|---|
| 🟠 Orange (solid) | Connecting to WiFi |
| 🟢 Green (solid) | Connected — ready |
| 🔴 Red (solid) | WiFi failed — check credentials |

---

## 🛠 Troubleshooting

| Problem | Fix |
|---|---|
| Lamp not appearing online | Check IP in Serial Monitor, re-enter in settings |
| Wrong colors (RGB vs GRB) | Change `COLOR_ORDER GRB` to `RGB` in firmware |
| Strip flickers | Add 330Ω resistor on data line, shorten wire |
| Compile error | Make sure FastLED library is installed |
| Dashboard blocked by browser | Use Chrome; allow mixed content if needed |
| SYNC ALL does nothing | Confirm laptop is on same WiFi as lamps |

---

## 🔢 Quick Reference — API Endpoints

Each lamp responds to simple HTTP GET requests:

```
GET http://<lamp-ip>/cmd?r=255&g=0&b=0&br=180&fx=0&sp=128&pw=1
GET http://<lamp-ip>/status
```

| Parameter | Range | Meaning |
|---|---|---|
| `r` `g` `b` | 0–255 | Red, Green, Blue |
| `br` | 0–255 | Brightness |
| `fx` | 0–3 | 0=Solid, 1=Rainbow, 2=Breathing, 3=Strobe |
| `sp` | 0–255 | Effect speed |
| `pw` | 0 / 1 | Power off / on |

---

*Start simple, test one lamp first, then scale to all 12!*
