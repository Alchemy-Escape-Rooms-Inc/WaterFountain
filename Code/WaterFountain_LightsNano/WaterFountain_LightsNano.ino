/*
 * ============================================
 * ALCHEMY ESCAPE ROOM - WATER FOUNTAIN LIGHTS (Arduino Nano)
 * ============================================
 * DUMB LIGHT DRIVER. The WaterFountain ESP32-S3 decides every color; this
 * Nano only paints what it is told, at a clean 5 V data level the WS2812B
 * strip can read (the S3's 3.3 V data was too weak).
 *
 * Nano has no OTA, so it holds NO game logic: flash once, never again.
 * All changes (colors, LED count, brightness, tests) happen on the S3 via OTA.
 *
 * WIRING:
 *   S3 GPIO39 (TX) -> Nano D2   (one way only; NEVER Nano -> S3, 5 V kills S3)
 *   S3 GND         -> Nano GND  (required)
 *   Nano D6        -> 330R -> strip DIN
 *   5 V supply     -> Nano 5V pin + strip 5V; supply GND -> Nano GND + strip GND
 *
 * PROTOCOL (9600 baud text, one line per frame, S3 resends every 500 ms):
 *   F,<ledsPerPot>,<brightness>,r1,g1,b1,r2,g2,b2,r3,g3,b3,<sum>\n
 *   sum = (all 11 numbers before it) mod 256. Bad/partial lines are ignored.
 *   FastLED.show() blocks serial, so the Nano only shows when a frame CHANGES;
 *   the S3's 500 ms gap means a show never overlaps an incoming frame.
 *
 * Boot: whole strip dim blue until the first valid frame arrives.
 * USB serial (115200) prints every accepted frame for debugging.
 * Boot: 1 s RED, 1 s GREEN, 1 s BLUE self-test on the whole strip, then dim blue.
 * v1.0.1 (2026-11-05) dummy TX D3 instead of -1; boot color self-test
 * v1.0.0 (2026-11-04)
 * ============================================
 */

#include <SoftwareSerial.h>
#include <FastLED.h>

#define VERSION       "1.0.1"
#define RX_PIN        2
#define LED_PIN       6
#define MAX_PER_POT   100          // 300 LEDs max (900 bytes RAM)
#define MAX_LEDS      (MAX_PER_POT * 3)

SoftwareSerial link(RX_PIN, 3);    // receive only; D3 = dummy TX (leave unwired). -1 made the library poke a bogus pin
CRGB leds[MAX_LEDS];

char line[96];
uint8_t lineLen = 0;
int lastVals[11] = { -1 };

void applyFrame(const int* v) {
  int perPot = constrain(v[0], 1, MAX_PER_POT);
  FastLED.setBrightness(constrain(v[1], 0, 255));
  fill_solid(leds, MAX_LEDS, CRGB::Black);
  for (int p = 0; p < 3; p++) {
    CRGB c(v[2 + p * 3], v[3 + p * 3], v[4 + p * 3]);
    for (int k = 0; k < perPot; k++) leds[p * perPot + k] = c;
  }
  FastLED.show();
}

void handleLine() {
  if (line[0] != 'F' || line[1] != ',') return;
  int v[12];
  int n = 0;
  char* tok = strtok(line + 2, ",");
  while (tok && n < 12) { v[n++] = atoi(tok); tok = strtok(NULL, ","); }
  if (n != 12) return;
  int sum = 0;
  for (int i = 0; i < 11; i++) sum += v[i];
  if ((sum & 0xFF) != v[11]) return;
  if (memcmp(v, lastVals, sizeof(lastVals)) == 0) return;   // unchanged, skip show
  memcpy(lastVals, v, sizeof(lastVals));
  applyFrame(v);
  Serial.print(F("[FRAME] per="));  Serial.print(v[0]);
  Serial.print(F(" bri="));         Serial.print(v[1]);
  for (int p = 0; p < 3; p++) {
    Serial.print(F(" pot")); Serial.print(p + 1); Serial.print('=');
    Serial.print(v[2 + p * 3]); Serial.print('/');
    Serial.print(v[3 + p * 3]); Serial.print('/');
    Serial.print(v[4 + p * 3]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("WaterFountain LightsNano v" VERSION));
  link.begin(9600);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, MAX_LEDS);
  FastLED.setBrightness(60);
  const CRGB test[3] = { CRGB::Red, CRGB::Green, CRGB::Blue };   // self-test: proves Nano->strip path with no S3
  for (int i = 0; i < 3; i++) { fill_solid(leds, MAX_LEDS, test[i]); FastLED.show(); delay(1000); }
  fill_solid(leds, MAX_LEDS, CRGB(0, 0, 255));   // dim blue = alive, waiting for S3
  FastLED.show();
}

void loop() {
  while (link.available()) {
    char c = link.read();
    if (c == '\n' || c == '\r') {
      if (lineLen) { line[lineLen] = '\0'; handleLine(); lineLen = 0; }
    } else if (lineLen < sizeof(line) - 1) {
      line[lineLen++] = c;
    } else {
      lineLen = 0;   // overflow = garbage, drop
    }
  }
}
