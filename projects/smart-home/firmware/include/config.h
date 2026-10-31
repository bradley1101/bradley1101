// Pin map and settings for the 3 nodes (ESP32 DevKit V1).
// Keep this file in sync with docs/wiring.md.
#pragma once

// ---------- Entrance node ----------
#define PIN_RFID_SS       5    // RC522 SDA
#define PIN_RFID_RST      22   // RC522 RST  (SCK 18, MOSI 23, MISO 19 = default VSPI)
#define PIN_DOOR_SERVO    13
#define PIN_LED_GRANTED   26   // green
#define PIN_LED_DENIED    27   // red
#define PIN_US_TRIG       32   // HC-SR04
#define PIN_US_ECHO       33   // HC-SR04 echo through a 5V -> 3.3V divider

#define DOOR_ANGLE_CLOSED 0
#define DOOR_ANGLE_OPEN   90
#define DOOR_OPEN_MS      5000  // door closes automatically after a badge
#define VISITOR_CM        15    // someone in front of the door below this distance

// Badge UIDs allowed to open the door (read them on the serial monitor)
#define AUTHORIZED_UIDS { "A1B2C3D4", "11223344" }

// ---------- Living room node ----------
#define PIN_PIR           27   // HC-SR501 output
#define PIN_LIGHT         25   // living room LEDs (220 ohm each)
#define LIGHT_HOLD_MS     10000 // light stays on 10 s after the last motion

// ---------- Kitchen node ----------
#define PIN_DHT           4    // DHT11 data
#define PIN_WINDOW_SERVO  13
#define PIN_LED_ALERT     26   // red alert LED

#define WINDOW_ANGLE_CLOSED 0
#define WINDOW_ANGLE_OPEN   90
#define TEMP_ALERT_C      35.0  // overheat threshold
#define TEMP_HYSTERESIS_C 2.0
#define CLIMATE_PERIOD_MS 5000

// ---------- Common ----------
#define PIN_STATUS_LED    2    // on-board LED: on when MQTT is connected
#define TOPIC_ROOT        "home"
