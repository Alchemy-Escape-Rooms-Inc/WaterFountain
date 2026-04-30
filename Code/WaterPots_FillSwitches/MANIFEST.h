/**
 * ============================================================================
 *  ALCHEMY ESCAPE ROOMS — FIRMWARE MANIFEST
 * ============================================================================
 *
 *  THIS FILE IS THE SINGLE SOURCE OF TRUTH.
 *
 *  It serves two masters simultaneously:
 *
 *    1. THE COMPILER — Every constant the firmware needs (pins, IPs, ports,
 *       thresholds, timing) is defined here as real C++ code. The firmware
 *       #includes this file and uses these values directly.
 *
 *    2. THE GRIMOIRE PARSER — A Python script running on M3 at 6 AM reads
 *       this file as plain text and extracts values tagged with @FIELD_NAME
 *       in the comments. Those values populate the WatchTower Grimoire
 *       device registry, wiring reference, and operations manual.
 *
 *  Because both systems read from the same lines, the documentation can
 *  never drift from the firmware. Change a pin number here, and the Grimoire
 *  updates automatically. There is no second file to keep in sync.
 *
 *  RULES:
 *    1. Every field marked [REQUIRED] must be filled in before deployment.
 *    2. Update this file FIRST when changing hardware, pins, or topics.
 *    3. The 6 AM parser looks for @TAG patterns — don't rename them.
 *    4. Descriptive-only sections (operations, quirks) are pure comments.
 *       Constants sections are real code + comment tags on the same line.
 *    5. This file is the sole source of configuration values — the .ino
 *       should reference these constants, not hardcode its own.
 *
 *  LAST UPDATED: 2026-04-29
 *  MANIFEST VERSION: 2.0
 * ============================================================================
 */

#pragma once
#include <cstdint>

// ============================================================================
//  SECTION 1 — IDENTITY
// ============================================================================
//
// @MANIFEST:IDENTITY
// @PROP_NAME:        WaterFountain
// @INSTANCE_COUNT:   1
//
// @DESCRIPTION:      Three ceremonial pots beneath a water-fountain prop.
//                    Players fill each pot with water from the fountain; a
//                    fill switch in each pot closes to GND when the pot
//                    contains enough water. When all three pots are filled
//                    simultaneously the puzzle is solved and PotsFilled
//                    publishes "true" (retained) to the broker. The
//                    accompanying servo-driven fountain levers are handled
//                    by a separate sketch (Water_Fountain.ino) — this
//                    sketch is sensors-only.
//
// @ROOM:             Mermaid's Tale — Water Fountain area
// @BOARD:            ESP32-S3
// @FRAMEWORK:        Arduino (arduino-cli, esp32:esp32:esp32s3)
// @REPO:             https://github.com/Alchemy-Escape-Rooms-Inc/WaterFountain
// @BUILD_STATUS:     stable
// @CODE_HEALTH:      good
// @WATCHTOWER:       full
// @END:IDENTITY

namespace manifest {

// ── Device Identity ─────────────────────────────────────────────────────────
inline constexpr const char* DEVICE_NAME      = "WaterFountain";   // @DEVICE_NAME      (MQTT client ID + topic base)
inline constexpr const char* FIRMWARE_VERSION = "1.0.0";           // @FIRMWARE_VERSION


// ============================================================================
//  SECTION 2 — NETWORK CONFIGURATION
// ============================================================================
// @MANIFEST:NETWORK

// ── WiFi ────────────────────────────────────────────────────────────────────
inline constexpr const char* WIFI_SSID     = "AlchemyGuest";        // @WIFI_SSID
inline constexpr const char* WIFI_PASSWORD = "VoodooVacation5601";  // @WIFI_PASS

// ── MQTT Broker ─────────────────────────────────────────────────────────────
inline constexpr const char* MQTT_SERVER   = "10.1.10.115";         // @BROKER_IP
inline constexpr int         MQTT_PORT     = 1883;                  // @BROKER_PORT
// MQTT Client ID: "WaterFountain_xxxx" (DEVICE_NAME + random hex suffix)

// ── Heartbeat ───────────────────────────────────────────────────────────────
inline constexpr unsigned long HEARTBEAT_INTERVAL = 300000;         // @HEARTBEAT_MS  (5 minutes, per WatchTower spec)

//  ── TOPIC MAP ──────────────────────────────────────────────────────────────
//
//  SUBSCRIPTIONS:
//  @SUBSCRIBE:  MermaidsTale/WaterFountain/command       | All commands
//
//  PUBLICATIONS:
//  @PUBLISH:  MermaidsTale/WaterFountain/command         | PONG, STATUS, OK responses     | retain:no
//  @PUBLISH:  MermaidsTale/WaterFountain/status          | ONLINE, HEARTBEAT              | retain:no
//  @PUBLISH:  MermaidsTale/WaterFountain/log             | Mirrored serial output         | retain:no
//  @PUBLISH:  MermaidsTale/WaterFountain/Pot1            | "filled" / "empty"             | retain:yes
//  @PUBLISH:  MermaidsTale/WaterFountain/Pot2            | "filled" / "empty"             | retain:yes
//  @PUBLISH:  MermaidsTale/WaterFountain/Pot3            | "filled" / "empty"             | retain:yes
//  @PUBLISH:  MermaidsTale/WaterFountain/PotsFilled      | "true" / "false" (all 3 in)    | retain:yes
//
//  NOTE: PONG and STATUS responses publish on /command (matches CoveDoor /
//        BarrelPiston pattern, differs from JungleDoor which uses /status).
//
//  SUPPORTED COMMANDS (via /command topic):
//  @COMMAND:  PING          | Responds PONG on /command topic                  | Health check
//  @COMMAND:  STATUS        | Sends state + uptime + RSSI + per-pot detail     | Full diagnostic
//  @COMMAND:  RESET         | Reboots ESP32                                    | Also accepts REBOOT, RESTART
//  @COMMAND:  PUZZLE_RESET  | Re-reads all 3 fill switches, republishes state  | No reboot
//
//  STATUS MESSAGES (published on /status topic):
//  @STATUS_MSG:  ONLINE          | Sent on boot and MQTT reconnect
//  @STATUS_MSG:  HEARTBEAT:...   | Periodic heartbeat with state, uptime, RSSI
//
//  POT STATE PAYLOADS (per-pot, on /Pot{1,2,3}):
//  @POT_PAYLOAD:  Waterpot N= filled  | Switch open / floating HIGH (water present)
//  @POT_PAYLOAD:  empty               | Switch closed to GND (no water)
//
//  AGGREGATE PAYLOAD (on /PotsFilled):
//  @AGG_PAYLOAD:  true            | All three pots simultaneously filled — puzzle solved
//  @AGG_PAYLOAD:  false           | One or more pots empty
//
// @END:NETWORK


// ============================================================================
//  SECTION 3 — PIN CONFIGURATION
// ============================================================================
// @MANIFEST:PINS

// ── Fill Switches (INPUT_PULLUP, active HIGH = filled) ──────────────────────
inline constexpr int PIN_POT1 = 4;                                  // @PIN:POT1   | Pot 1 fill switch — closes to GND when EMPTY (HIGH = filled)
inline constexpr int PIN_POT2 = 5;                                  // @PIN:POT2   | Pot 2 fill switch — closes to GND when EMPTY (HIGH = filled)
inline constexpr int PIN_POT3 = 6;                                  // @PIN:POT3   | Pot 3 fill switch — closes to GND when EMPTY (HIGH = filled)

// @END:PINS


// ============================================================================
//  SECTION 4 — SENSOR THRESHOLDS
// ============================================================================
// @MANIFEST:THRESHOLDS

// ── Debounce ────────────────────────────────────────────────────────────────
inline constexpr unsigned long DEBOUNCE_MS = 50;                    // @DEBOUNCE:POT  | 50ms debounce per fill switch

// @END:THRESHOLDS


// ============================================================================
//  SECTION 5 — TIMING CONSTANTS
// ============================================================================
// @MANIFEST:TIMING

inline constexpr unsigned long MQTT_RECONNECT_INTERVAL = 5000;      // @TIMING:MQTT_RECONNECT | Retry MQTT connection every 5s

// @END:TIMING

} // namespace manifest


// ============================================================================
//  SECTION 6 — COMPONENTS
// ============================================================================
//
// @MANIFEST:COMPONENTS
//
// @COMPONENT:  ESP32-S3 DevKit
//   @PURPOSE:  Reads three fill switches, publishes per-pot and aggregate
//              state to MQTT broker.
//   @DETAIL:   Native USB CDC for serial / programming. Powered via USB.
//
// @COMPONENT:  Fill Switch (Pot 1)
//   @PURPOSE:  Detects water presence in Pot 1
//   @DETAIL:   GPIO 4, INPUT_PULLUP, active HIGH. Switch closes to GND when
//              the pot is EMPTY; water lifts the float and opens the contact,
//              letting the pullup float HIGH = filled.
//
// @COMPONENT:  Fill Switch (Pot 2)
//   @PURPOSE:  Detects water presence in Pot 2
//   @DETAIL:   GPIO 5, INPUT_PULLUP, active HIGH (LOW = empty, HIGH = filled).
//
// @COMPONENT:  Fill Switch (Pot 3)
//   @PURPOSE:  Detects water presence in Pot 3
//   @DETAIL:   GPIO 6, INPUT_PULLUP, active HIGH (LOW = empty, HIGH = filled).
//
// @END:COMPONENTS


// ============================================================================
//  SECTION 7 — OPERATIONS
// ============================================================================
//
// @MANIFEST:OPERATIONS
//
//  ── PHYSICAL LOCATION ──────────────────────────────────────────────────────
//
// @LOCATION:  ESP32-S3 mounted near the WaterFountain prop. Fill switch
//             leads run from each pot back to the board. Common GND shared
//             across all three switches.
//
//  ── RESET PROCEDURES ───────────────────────────────────────────────────────
//
// @RESET:SOFTWARE
//   Send "RESET" (or "REBOOT" or "RESTART") to MermaidsTale/WaterFountain/command
//   Device responds "OK" on /command, then reboots.
//   After reboot: reconnects WiFi, reconnects MQTT, re-reads all 3 fill
//   switches, republishes Pot1/Pot2/Pot3 (retained) and PotsFilled (retained),
//   publishes ONLINE.
//   Expected recovery time: 10-15 seconds.
//
// @RESET:PUZZLE
//   Send "PUZZLE_RESET" to MermaidsTale/WaterFountain/command
//   Re-reads each fill switch and republishes its current state. Recomputes
//   PotsFilled from the fresh reads. No reboot, no WiFi/MQTT reconnect.
//   Responds "OK" on /command topic.
//   Use between game sessions to drain pots and republish "false" /
//   "empty" without restarting the device.
//
// @RESET:HARDWARE
//   Power-cycle the ESP32-S3 via its USB cable. Retained Pot{1,2,3} and
//   PotsFilled topics are republished from the live switch reads on boot.
//
//  ── TEST PROCEDURE ─────────────────────────────────────────────────────────
//
// @TEST:STEP1  Send PING to /command → expect PONG on /command (confirms MQTT)
// @TEST:STEP2  Send STATUS to /command → expect state, uptime, RSSI, version,
//              Pot1/Pot2/Pot3 state on /command
// @TEST:STEP3  Manually close Pot1 fill switch → expect "filled" on /Pot1 (retained)
// @TEST:STEP4  Repeat for Pot2 and Pot3 → expect each /Pot{n} to publish "filled"
// @TEST:STEP5  With all three closed → expect "true" on /PotsFilled (retained)
// @TEST:STEP6  Open any one switch → expect "empty" on that /Pot{n} AND
//              "false" on /PotsFilled
// @TEST:STEP7  Send PUZZLE_RESET → expect OK on /command, current state of
//              each pot republished
// @TEST:STEP8  Wait 5 minutes idle → expect HEARTBEAT on /status
//
//  ── KNOWN QUIRKS ───────────────────────────────────────────────────────────
//
// @QUIRK:S3_USB_CDC
//   ESP32-S3 native USB CDC can drop the COM port during esptool's reset
//   sequence. If a flash fails mid-upload, simply retry; if the port
//   doesn't re-enumerate, replug the USB cable. Build cache stays valid
//   across retries.
//
// @QUIRK:STRAPPING_PINS
//   On the ESP32-S3, GPIO 0, 3, 45, 46 are strapping pins. Pots are wired
//   to GPIO 4, 5, 6 — none of which are strapping, all of which have working
//   internal pullups. Do not move pots to strapping pins; boot-loops result.
//
// @QUIRK:TOPIC_COPY_FIRST
//   PubSubClient's topic pointer can be reused before the callback returns.
//   mqttCallback() copies topic into a local buffer FIRST, then declares
//   other locals. Do not reorder this — stack corruption results.
//
// @QUIRK:RETAINED_AGGREGATE
//   /PotsFilled is retained so late-joining subscribers (e.g. the Unreal
//   Engine game client) see the current solved/unsolved state immediately
//   on connect, without having to wait for the next switch transition.
//
// @END:OPERATIONS


// ============================================================================
//  SECTION 8 — DEPENDENCIES
// ============================================================================
//
// @MANIFEST:DEPENDENCIES
//
// @LIB:  WiFi              | ESP32 WiFi driver          | Built-in (esp32:esp32 core 3.x)
// @LIB:  PubSubClient      | MQTT client                | v2.8+
//
// @END:DEPENDENCIES


// ============================================================================
//  SECTION 9 — WIRING SUMMARY
// ============================================================================
//
// @MANIFEST:WIRING
//
//   ESP32-S3 GPIO 4 ──────────── Pot 1 fill switch ──── GND
//   ESP32-S3 GPIO 5 ──────────── Pot 2 fill switch ──── GND
//   ESP32-S3 GPIO 6 ──────────── Pot 3 fill switch ──── GND
//
//   Each pin uses INPUT_PULLUP. Switch closes to GND when the pot is empty;
//   water lifts the float and opens the contact, letting the pullup float
//   HIGH: digitalRead == HIGH means "filled", LOW means "empty".
//
//   ESP32-S3 USB ────────────── Power + programming + serial log
//
// @END:WIRING
