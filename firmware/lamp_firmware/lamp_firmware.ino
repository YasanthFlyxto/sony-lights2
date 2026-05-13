/*
 * =====================================================
 *  RGB Lamp Firmware  |  ESP32 + WS2812B
 *  Only ONE external library needed: FastLED
 *  Built-in: WiFi.h, WebServer.h  (no extra installs)
 * =====================================================
 *
 *  CHANGE THESE 3 LINES BEFORE FLASHING EACH ESP32:
 */
const char* WIFI_SSID = "Eagle home fiber" ; //-- your WiFi name
const char* WIFI_PASS = "ge2262809" ; //-- WiFi password
const int   LAMP_ID   = 10;               // <-- 1 to 12
/*
 * =====================================================
 */

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>
#include <WebSocketsServer.h> // <-- Add this (Install "WebSockets" by Markus Sattler)
#include <FastLED.h>
#include <Preferences.h>

// ── LED Strip Settings ────────────────────────────────
#define LED_PIN      5       // GPIO5 → WS2812B Data pin
#define NUM_LEDS     30      // Number of LEDs per strip
#define LED_TYPE     WS2812B
#define COLOR_ORDER  GRB

CRGB leds[NUM_LEDS];
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81); // WebSocket on port 81
WiFiMulti wifiMulti;
Preferences prefs;

// ── Current State (Target Values) ─────────────────────
uint8_t  targetR = 255, targetG = 80, targetB = 0;
uint8_t  brightness = 180;
uint8_t  effect = 0;   // 0=Solid 1=Rainbow 2=Breathing 3=Strobe
uint8_t  spd    = 128;
bool     power  = true;
bool     dirty  = true;

// ── Effect Variables (Hardware Fader) ─────────────────
uint8_t       currentBri  = 180;
uint8_t       curR = 255, curG = 80, curB = 0;
unsigned long lastFade    = 0;
uint8_t       rainbowHue  = 0;
float         breathAngle = 0.0f;
bool          strobeState = false;
unsigned long lastFx      = 0;

// ── Helper: send response with CORS headers ───────────
void sendOK(const String& body = "OK", const String& type = "text/plain") {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, type, body);
}

// ── /cmd  — receive color/effect commands ─────────────
void handleCmd() {
  if (server.hasArg("r"))  { targetR = constrain(server.arg("r").toInt(), 0, 255); dirty = true; }
  if (server.hasArg("g"))  { targetG = constrain(server.arg("g").toInt(), 0, 255); dirty = true; }
  if (server.hasArg("b"))  { targetB = constrain(server.arg("b").toInt(), 0, 255); dirty = true; }
  if (server.hasArg("br")) { brightness = constrain(server.arg("br").toInt(), 0, 255); dirty = true; }
  if (server.hasArg("fx")) { effect = constrain(server.arg("fx").toInt(), 0, 3); dirty = true; }
  if (server.hasArg("sp")) { spd = constrain(server.arg("sp").toInt(), 0, 255); dirty = true; }
  if (server.hasArg("pw")) { power = server.arg("pw").toInt() != 0; dirty = true; }
  sendOK();
}

// ── /status — report current state as JSON ────────────
void handleStatus() {
  String json = "{\"id\":" + String(LAMP_ID)
              + ",\"ip\":\"" + WiFi.localIP().toString() + "\""
              + ",\"r\":"  + targetR
              + ",\"g\":"  + targetG
              + ",\"b\":"  + targetB
              + ",\"br\":" + brightness
              + ",\"fx\":" + effect
              + ",\"sp\":" + spd
              + ",\"pw\":" + (power ? 1 : 0)
              + "}";
  sendOK(json, "application/json");
}

// ── /identify — visually identify lamp ────────────────
void handleIdentify() {
  sendOK();
  uint8_t oldBri = brightness;
  for (int i=0; i<3; i++) {
    FastLED.setBrightness(255);
    fill_solid(leds, NUM_LEDS, CRGB::White);
    FastLED.show();
    delay(150);
    FastLED.setBrightness(0);
    FastLED.show();
    delay(150);
  }
  FastLED.setBrightness(oldBri);
  dirty = true;
}

// ── WebSocket Event Handler ──────────────────────────
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
        // Send current status to the new client
        handleStatus(); // This still works for HTTP, but we want to push to WS
      }
      break;
    case WStype_TEXT: {
        String msg = String((char*)payload);
        // Simple comma-separated or JSON-like parser
        // Format expected: "r:255,g:10,b:0,br:255,fx:0,sp:128,pw:1"
        if (msg.indexOf("r:") != -1)  targetR = msg.substring(msg.indexOf("r:") + 2).toInt();
        if (msg.indexOf("g:") != -1)  targetG = msg.substring(msg.indexOf("g:") + 2).toInt();
        if (msg.indexOf("b:") != -1)  targetB = msg.substring(msg.indexOf("b:") + 2).toInt();
        if (msg.indexOf("br:") != -1) brightness = msg.substring(msg.indexOf("br:") + 3).toInt();
        if (msg.indexOf("fx:") != -1) effect = msg.substring(msg.indexOf("fx:") + 3).toInt();
        if (msg.indexOf("sp:") != -1) spd = msg.substring(msg.indexOf("sp:") + 3).toInt();
        if (msg.indexOf("pw:") != -1) power = msg.substring(msg.indexOf("pw:") + 3).toInt() != 0;
        dirty = true;
        break;
      }
  }
}

// ── Broadcast Status to all WS clients ────────────────
void broadcastStatus() {
  String json = "{\"id\":" + String(LAMP_ID)
              + ",\"ip\":\"" + WiFi.localIP().toString() + "\""
              + ",\"r\":"  + targetR
              + ",\"g\":"  + targetG
              + ",\"b\":"  + targetB
              + ",\"br\":" + brightness
              + ",\"fx\":" + effect
              + ",\"sp\":" + spd
              + ",\"pw\":" + (power ? 1 : 0)
              + "}";
  webSocket.broadcastTXT(json);
}

// ── CORS preflight ────────────────────────────────────
void handleOptions() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  server.send(204);
}

// ── LED Effects (non-blocking) ────────────────────────
void runEffect() {
  unsigned long now = millis();

  // Hardware Fade Engine (Brightness & Color)
  uint8_t targetBri = power ? brightness : 0;
  bool colorFading = (curR != targetR) || (curG != targetG) || (curB != targetB);
  
  if (currentBri != targetBri || colorFading) {
    if (now - lastFade >= 3) { // ~3ms per step = ~765ms full fade. Very smooth.
      if (currentBri < targetBri) currentBri++;
      else if (currentBri > targetBri) currentBri--;
      
      if (curR < targetR) curR++; else if (curR > targetR) curR--;
      if (curG < targetG) curG++; else if (curG > targetG) curG--;
      if (curB < targetB) curB++; else if (curB > targetB) curB--;

      lastFade = now;
      FastLED.setBrightness(currentBri);
      
      // If solid color, update leds array continuously during fade
      if (effect == 0) {
        fill_solid(leds, NUM_LEDS, CRGB(curR, curG, curB));
      }
      FastLED.show();
    }
  }

  // If fully off, don't waste CPU rendering effects
  if (currentBri == 0) return;

  switch (effect) {
    case 0: { // Solid
      // Fading logic handles the show() call above.
      // But we still need to catch non-fading dirty updates (like FX changes)
      if (dirty && !colorFading && currentBri == targetBri) {
        FastLED.setBrightness(currentBri);
        fill_solid(leds, NUM_LEDS, CRGB(curR, curG, curB));
        FastLED.show();
        dirty = false;
      }
      break;
    }
    case 1: { // Rainbow
      uint16_t fxDelay = map(spd, 0, 255, 120, 5);
      if (now - lastFx >= fxDelay) {
        FastLED.setBrightness(currentBri);
        fill_rainbow(leds, NUM_LEDS, rainbowHue++, 255 / NUM_LEDS);
        FastLED.show();
        lastFx = now;
      }
      break;
    }
    case 2: { // Breathing
      uint16_t fxDelay = map(spd, 0, 255, 25, 5);
      if (now - lastFx >= fxDelay) {
        breathAngle += 0.04f;
        if (breathAngle > TWO_PI) breathAngle -= TWO_PI;
        // Breathing calculates its own brightness, bounded by currentBri
        uint8_t bri = (uint8_t)((sinf(breathAngle) + 1.0f) * 0.5f * currentBri);
        FastLED.setBrightness(bri);
        fill_solid(leds, NUM_LEDS, CRGB(curR, curG, curB));
        FastLED.show();
        lastFx = now;
      }
      break;
    }
    case 3: { // Strobe
      uint16_t fxDelay = map(spd, 0, 255, 300, 20);
      if (now - lastFx >= fxDelay) {
        strobeState = !strobeState;
        FastLED.setBrightness(currentBri);
        fill_solid(leds, NUM_LEDS, strobeState ? CRGB(curR, curG, curB) : CRGB::Black);
        FastLED.show();
        lastFx = now;
      }
      break;
    }
  }
}

// ── /getwifi — get currently saved custom credentials ───
void handleGetWifi() {
  prefs.begin("wifi", true);
  String json = "[";
  bool first = true;
  for (int i=0; i<5; i++) {
    String s = prefs.getString(("ssid_" + String(i)).c_str(), "");
    if (s.length() > 0) {
      if (!first) json += ",";
      json += "{\"index\":" + String(i) + ",\"ssid\":\"" + s + "\"}";
      first = false;
    }
  }
  json += "]";
  prefs.end();
  sendOK(json, "application/json");
}

// ── /clearwifi — remove a specific saved credential ──────
void handleClearWifi() {
  if (server.hasArg("index")) {
    int idx = server.arg("index").toInt();
    if (idx >= 0 && idx < 5) {
      prefs.begin("wifi", false);
      prefs.remove(("ssid_" + String(idx)).c_str());
      prefs.remove(("pass_" + String(idx)).c_str());
      prefs.end();
      sendOK("Cleared slot " + String(idx) + ". Rebooting...");
      delay(500);
      ESP.restart();
      return;
    }
  }
  server.send(400, "text/plain", "Missing or invalid index (0-4)");
}

// ── /setwifi — save new credentials to an empty slot ─────
void handleSetWifi() {
  if (server.hasArg("ssid") && server.hasArg("pass")) {
    prefs.begin("wifi", false);
    int slot = -1;
    for (int i=0; i<5; i++) {
      if (prefs.getString(("ssid_" + String(i)).c_str(), "").length() == 0) {
        slot = i;
        break;
      }
    }
    if (slot == -1) {
      prefs.end();
      server.send(400, "text/plain", "Memory full! Delete a network first.");
      return;
    }
    prefs.putString(("ssid_" + String(slot)).c_str(), server.arg("ssid"));
    prefs.putString(("pass_" + String(slot)).c_str(), server.arg("pass"));
    prefs.end();
    sendOK("Saved to slot " + String(slot) + ". Rebooting...");
    delay(500);
    ESP.restart();
  } else {
    server.send(400, "text/plain", "Missing ssid or pass");
  }
}

// ── Setup ─────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(200);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness);

  // Orange = connecting
  fill_solid(leds, NUM_LEDS, CRGB(255, 80, 0));
  FastLED.show();

  prefs.begin("wifi", true);
  String ssids[5];
  String passes[5];
  for (int i=0; i<5; i++) {
    ssids[i] = prefs.getString(("ssid_" + String(i)).c_str(), "");
    passes[i] = prefs.getString(("pass_" + String(i)).c_str(), "");
  }
  prefs.end();

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); // <-- CRITICAL: Prevents WiFi jitter/latency spikes
  
  // Add hardcoded network
  wifiMulti.addAP(WIFI_SSID, WIFI_PASS);
  
  // Add custom networks
  for (int i=0; i<5; i++) {
    if (ssids[i].length() > 0) {
      wifiMulti.addAP(ssids[i].c_str(), passes[i].c_str());
      Serial.printf("\nLoaded custom network [%d]: %s", i, ssids[i].c_str());
    }
  }

  Serial.printf("\nLamp %d scanning for known networks...", LAMP_ID);

  int tries = 0;
  while (wifiMulti.run() != WL_CONNECTED && tries < 30) {
    delay(500);
    Serial.print(".");
    tries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n✅ Lamp %d online at: http://%s\n", LAMP_ID, WiFi.localIP().toString().c_str());
    // Green = connected
    fill_solid(leds, NUM_LEDS, CRGB(0, 220, 0));
    FastLED.show();
    delay(1500);
  } else {
    Serial.println("\n❌ WiFi failed. Check SSID/Password.");
    // Red = failed
    fill_solid(leds, NUM_LEDS, CRGB(255, 0, 0));
    FastLED.show();
    delay(2000);
  }

  server.on("/cmd",       HTTP_GET,     handleCmd);
  server.on("/status",    HTTP_GET,     handleStatus);
  server.on("/identify",  HTTP_GET,     handleIdentify);
  server.on("/setwifi",   HTTP_GET,     handleSetWifi);
  server.on("/getwifi",   HTTP_GET,     handleGetWifi);
  server.on("/clearwifi", HTTP_GET,     handleClearWifi);
  
  server.on("/cmd",       HTTP_OPTIONS, handleOptions);
  server.on("/status",    HTTP_OPTIONS, handleOptions);
  server.on("/identify",  HTTP_OPTIONS, handleOptions);
  server.on("/setwifi",   HTTP_OPTIONS, handleOptions);
  server.on("/getwifi",   HTTP_OPTIONS, handleOptions);
  server.on("/clearwifi", HTTP_OPTIONS, handleOptions);
  server.begin();
  
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  
  Serial.println("HTTP and WebSocket servers started.");
}

// ── Loop ──────────────────────────────────────────────
void loop() {
  server.handleClient();
  webSocket.loop();
  runEffect();
  
  // Every time something is 'dirty' (changed), broadcast to all WS clients
  if (dirty) {
    broadcastStatus();
    dirty = false; // Clear flag after broadcast to prevent flooding
  }
}
