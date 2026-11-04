/*
 * ============================================
 * ALCHEMY ESCAPE ROOM - WATER FOUNTAIN POTS
 * ESP32-S3 + 3 Fill Switches (to GND)
 * Compliant with WatchTower Coding Standards v2025-12-21
 * ============================================
 *
 * Each switch closes to GND when its pot is empty.
 * Wired with INPUT_PULLUP — LOW = empty, HIGH = filled.
 *
 * MQTT TOPICS:
 *   Subscribe: MermaidsTale/WaterFountain/command
 *   Publish:   MermaidsTale/WaterFountain/command   (PONG, STATUS, OK responses)
 *   Publish:   MermaidsTale/WaterFountain/status    (ONLINE, HEARTBEAT)
 *   Publish:   MermaidsTale/WaterFountain/log       (debug output)
 *   Publish:   MermaidsTale/WaterFountain/Pot1      (filled / empty, retained)
 *   Publish:   MermaidsTale/WaterFountain/Pot2      (filled / empty, retained)
 *   Publish:   MermaidsTale/WaterFountain/Pot3      (filled / empty, retained)
 *   Publish:   MermaidsTale/WaterFountain/PotsFilled (true / false, retained)
 *
 * v1.1.0 (owner 2026-11-04): WS2812B strip on GPIO39 (5V from the fountain
 * supply, common ground). Strip is split into three sections, one per pot,
 * in strip order Pot1, Pot2, Pot3: BLUE = empty, ORANGE = filled.
 * Also adds mandatory ArduinoOTA (hostname WaterFountain, port 3232,
 * password = Wi-Fi) - after this USB flash every update is wireless.
 * ============================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <stdarg.h>
#include <ArduinoOTA.h>
#include <FastLED.h>

#define VERSION    "1.1.0"
#define GAME_NAME  "MermaidsTale"
#define PROP_NAME  "WaterFountain"

// ── Network ────────────────────────────────────────────────────────────────
const char* WIFI_SSID     = "AlchemyGuest";
const char* WIFI_PASSWORD = "VoodooVacation5601";
const char* MQTT_SERVER   = "10.1.10.115";
const int   MQTT_PORT     = 1883;

// ── MQTT Topics ────────────────────────────────────────────────────────────
#define MQTT_TOPIC_COMMAND      "MermaidsTale/WaterFountain/command"
#define MQTT_TOPIC_STATUS       "MermaidsTale/WaterFountain/status"
#define MQTT_TOPIC_LOG          "MermaidsTale/WaterFountain/log"
#define MQTT_TOPIC_POT1         "MermaidsTale/WaterFountain/Pot1"
#define MQTT_TOPIC_POT2         "MermaidsTale/WaterFountain/Pot2"
#define MQTT_TOPIC_POT3         "MermaidsTale/WaterFountain/Pot3"
#define MQTT_TOPIC_POTS_FILLED  "MermaidsTale/WaterFountain/PotsFilled"

// ── Pins (ESP32-S3, non-strapping with working pullups) ────────────────────
const int PIN_POT1 = 4;
const int PIN_POT2 = 5;
const int PIN_POT3 = 6;

// -- Lights (WS2812B) --------------------------------------------------------
// EDIT LEDS_PER_POT to match the strip: total LEDs / 3. Order along the
// strip from the data-in end = Pot1, Pot2, Pot3.
#define LED_DATA_PIN    39
#define LEDS_PER_POT    10
#define LED_COUNT       (LEDS_PER_POT * 3)
#define LED_BRIGHTNESS  180          // 0-255
CRGB leds[LED_COUNT];
static const CRGB COLOR_EMPTY  = CRGB(0, 0, 255);     // blue
static const CRGB COLOR_FILLED = CRGB(255, 90, 0);    // orange
bool lightsDirty = true;

// ── Timing ─────────────────────────────────────────────────────────────────
const unsigned long DEBOUNCE_MS             = 50;
const unsigned long HEARTBEAT_INTERVAL      = 300000;   // 5 min
const unsigned long MQTT_RECONNECT_INTERVAL = 5000;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

struct Pot {
  int pin;
  const char* topic;
  const char* name;
  bool filled;            // debounced state (true = HIGH = filled)
  bool lastRawFilled;
  unsigned long lastChangeMs;
};

Pot pots[3] = {
  { PIN_POT1, MQTT_TOPIC_POT1, "Waterpot 1", false, false, 0 },
  { PIN_POT2, MQTT_TOPIC_POT2, "Waterpot 2", false, false, 0 },
  { PIN_POT3, MQTT_TOPIC_POT3, "Waterpot 3", false, false, 0 },
};

bool allPotsFilledLast = false;

void renderLights() {
  for (int i = 0; i < 3; i++) {
    CRGB c = pots[i].filled ? COLOR_FILLED : COLOR_EMPTY;
    for (int k = 0; k < LEDS_PER_POT; k++) leds[i * LEDS_PER_POT + k] = c;
  }
  FastLED.show();
  lightsDirty = false;
}

// Boot + LIGHTS_TEST: whole strip RED, GREEN, BLUE (proves wiring + color order).
void lightsSelfTest() {
  const CRGB seq[3] = { CRGB(255, 0, 0), CRGB(0, 255, 0), CRGB(0, 0, 255) };
  for (int k = 0; k < 3; k++) {
    fill_solid(leds, LED_COUNT, seq[k]);
    FastLED.show();
    delay(300);
  }
  lightsDirty = true;
}
unsigned long lastHeartbeat = 0;
unsigned long lastMqttReconnect = 0;

// ── Logging helper (publishes to /log + Serial) ────────────────────────────
void mqttLogf(const char* format, ...) {
  char buffer[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  Serial.println(buffer);
  if (mqttClient.connected()) {
    mqttClient.publish(MQTT_TOPIC_LOG, buffer);
  }
}

// ── Per-pot publish ────────────────────────────────────────────────────────
// Asymmetric payload per spec:
//   filled -> "Waterpot N= filled"   (carries pot identity)
//   empty  -> "empty"                (bare word, identity comes from topic)
static void formatPotPayload(const Pot& p, char* buf, size_t len) {
  if (p.filled) {
    snprintf(buf, len, "%s= filled", p.name);
  } else {
    snprintf(buf, len, "empty");
  }
}

void publishPot(const Pot& p) {
  lightsDirty = true;
  char payload[32];
  formatPotPayload(p, payload, sizeof(payload));
  mqttLogf("[POT] %s = %s", p.name, p.filled ? "filled" : "empty");
  if (mqttClient.connected()) {
    mqttClient.publish(p.topic, payload, true);   // retained
  }
}

void publishPotsFilled(bool all) {
  const char* payload = all ? "true" : "false";
  mqttLogf("[ALL] PotsFilled=%s", payload);
  if (mqttClient.connected()) {
    mqttClient.publish(MQTT_TOPIC_POTS_FILLED, payload, true);  // retained
  }
}

// ── Puzzle reset (re-read switches, republish current state) ───────────────
void puzzleReset() {
  for (int i = 0; i < 3; i++) {
    Pot& p = pots[i];
    bool rawFilled = (digitalRead(p.pin) == HIGH);
    p.filled = rawFilled;
    p.lastRawFilled = rawFilled;
    p.lastChangeMs = millis();
    publishPot(p);
  }
  allPotsFilledLast = pots[0].filled && pots[1].filled && pots[2].filled;
  publishPotsFilled(allPotsFilledLast);
}

// ── MQTT callback (CRITICAL: copy topic FIRST per WatchTower spec) ─────────
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Copy topic to local buffer FIRST to prevent stack corruption
  char topicBuf[128];
  strncpy(topicBuf, topic, sizeof(topicBuf) - 1);
  topicBuf[sizeof(topicBuf) - 1] = '\0';

  // Now safe to declare other locals
  char message[128];
  if (length >= sizeof(message)) length = sizeof(message) - 1;
  memcpy(message, payload, length);
  message[length] = '\0';

  // Trim whitespace
  char* msg = message;
  while (*msg == ' ' || *msg == '\t' || *msg == '\r' || *msg == '\n') msg++;
  char* end = msg + strlen(msg) - 1;
  while (end > msg && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) {
    *end = '\0';
    end--;
  }

  Serial.printf("[MQTT] Received on %s: %s\n", topicBuf, msg);

  if (strcmp(topicBuf, MQTT_TOPIC_COMMAND) != 0) return;

  // PING → PONG
  if (strcmp(msg, "PING") == 0) {
    mqttClient.publish(MQTT_TOPIC_COMMAND, "PONG");
    Serial.println("[MQTT] PING -> PONG");
    return;
  }

  // STATUS → current state report (state, uptime, RSSI, version + pot detail)
  if (strcmp(msg, "STATUS") == 0) {
    char buf[192];
    const char* state = allPotsFilledLast ? "ALL_FILLED" : "WAITING";
    snprintf(buf, sizeof(buf),
             "STATUS:%s:UP%lus:RSSI%d:v%s:IP:%s:OTA:3232:Pot1=%s:Pot2=%s:Pot3=%s",
             state, millis() / 1000, WiFi.RSSI(), VERSION, WiFi.localIP().toString().c_str(),
             pots[0].filled ? "filled" : "empty",
             pots[1].filled ? "filled" : "empty",
             pots[2].filled ? "filled" : "empty");
    mqttClient.publish(MQTT_TOPIC_COMMAND, buf);
    Serial.printf("[MQTT] STATUS -> %s\n", buf);
    return;
  }

  // RESET → reboot
  if (strcmp(msg, "RESET") == 0 || strcmp(msg, "REBOOT") == 0 || strcmp(msg, "RESTART") == 0) {
    mqttClient.publish(MQTT_TOPIC_COMMAND, "OK");
    Serial.println("[MQTT] RESET -> Rebooting...");
    delay(200);
    ESP.restart();
    return;
  }

  // PUZZLE_RESET → re-read switches, no reboot
  if (strcmp(msg, "PUZZLE_RESET") == 0) {
    puzzleReset();
    mqttClient.publish(MQTT_TOPIC_COMMAND, "OK");
    Serial.println("[MQTT] PUZZLE_RESET -> OK");
    return;
  }

  // LIGHTS_TEST -> R/G/B sweep, then back to pot colors
  if (strcmp(msg, "LIGHTS_TEST") == 0) {
    mqttClient.publish(MQTT_TOPIC_COMMAND, "OK");
    lightsSelfTest();
    return;
  }

  Serial.printf("[MQTT] Unknown command: %s\n", msg);
}

// ── WiFi ───────────────────────────────────────────────────────────────────
void setup_wifi() {
  Serial.printf("[WIFI] Connecting to %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\n[WIFI] Connected, IP=%s\n", WiFi.localIP().toString().c_str());
}

// ── MQTT reconnect ─────────────────────────────────────────────────────────
void mqtt_reconnect() {
  if (mqttClient.connected()) return;
  unsigned long now = millis();
  if (now - lastMqttReconnect < MQTT_RECONNECT_INTERVAL) return;
  lastMqttReconnect = now;

  String clientId = PROP_NAME;
  clientId += "_";
  clientId += String(random(0xffff), HEX);

  Serial.printf("[MQTT] Connecting as %s...", clientId.c_str());
  if (mqttClient.connect(clientId.c_str())) {
    Serial.println(" connected");
    mqttClient.subscribe(MQTT_TOPIC_COMMAND);
    mqttClient.publish(MQTT_TOPIC_STATUS, "ONLINE");
    mqttLogf("%s v%s online at IP %s (OTA :3232)", PROP_NAME, VERSION,
             WiFi.localIP().toString().c_str());
    // Republish current pot state on reconnect
    for (int i = 0; i < 3; i++) {
      char payload[32];
      formatPotPayload(pots[i], payload, sizeof(payload));
      mqttClient.publish(pots[i].topic, payload, true);
    }
    mqttClient.publish(MQTT_TOPIC_POTS_FILLED, allPotsFilledLast ? "true" : "false", true);
  } else {
    Serial.printf(" failed rc=%d\n", mqttClient.state());
  }
}

// ── Read switches with debounce ────────────────────────────────────────────
void readPots() {
  unsigned long now = millis();
  for (int i = 0; i < 3; i++) {
    Pot& p = pots[i];
    bool rawFilled = (digitalRead(p.pin) == HIGH);

    if (rawFilled != p.lastRawFilled) {
      p.lastChangeMs = now;
      p.lastRawFilled = rawFilled;
    }

    if ((now - p.lastChangeMs) >= DEBOUNCE_MS && rawFilled != p.filled) {
      p.filled = rawFilled;
      publishPot(p);
    }
  }

  bool allFilled = pots[0].filled && pots[1].filled && pots[2].filled;
  if (allFilled != allPotsFilledLast) {
    allPotsFilledLast = allFilled;
    publishPotsFilled(allFilled);
  }
}

// ── Heartbeat ──────────────────────────────────────────────────────────────
void send_heartbeat() {
  unsigned long now = millis();
  if (now - lastHeartbeat < HEARTBEAT_INTERVAL) return;
  lastHeartbeat = now;
  if (!mqttClient.connected()) return;

  char buf[160];
  const char* state = allPotsFilledLast ? "ALL_FILLED" : "WAITING";
  snprintf(buf, sizeof(buf),
           "HEARTBEAT:%s:UP%lus:RSSI%d",
           state, now / 1000, WiFi.RSSI());
  mqttClient.publish(MQTT_TOPIC_STATUS, buf);
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.printf("\n=== %s v%s — Fill Switches ===\n", PROP_NAME, VERSION);

  pinMode(PIN_POT1, INPUT_PULLUP);
  pinMode(PIN_POT2, INPUT_PULLUP);
  pinMode(PIN_POT3, INPUT_PULLUP);

  // Seed initial state so first reading publishes if already filled at boot
  for (int i = 0; i < 3; i++) {
    pots[i].lastRawFilled = !(digitalRead(pots[i].pin) == HIGH);
    pots[i].lastChangeMs = millis();
  }

  FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, LED_COUNT);
  FastLED.setBrightness(LED_BRIGHTNESS);
  lightsSelfTest();

  setup_wifi();

  // Mandatory OTA (fleet protocol): hostname = device, password = Wi-Fi
  ArduinoOTA.setHostname(PROP_NAME);
  ArduinoOTA.setPassword(WIFI_PASSWORD);
  ArduinoOTA.onStart([]() { mqttLogf("OTA update starting - back in ~30 s"); });
  ArduinoOTA.begin();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(512);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    delay(500);
    return;
  }
  ArduinoOTA.handle();
  if (!mqttClient.connected()) mqtt_reconnect();
  mqttClient.loop();

  readPots();
  if (lightsDirty) renderLights();
  send_heartbeat();
  delay(10);
}
