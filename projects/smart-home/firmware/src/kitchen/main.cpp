// Kitchen node — temperature/humidity monitoring, overheat alert, ventilation window.
//
// Publishes : home/kitchen/climate        {"t":28.0,"h":61.0}
//             home/kitchen/alert          overheat | normal    (retained)
//             home/kitchen/window/state   open | closed        (retained)
//             home/kitchen/window/mode    auto | manual        (retained)
// Listens   : home/kitchen/window/set     open | close | auto
//   In auto mode the window opens when the temperature exceeds TEMP_ALERT_C
//   and closes once it drops below TEMP_ALERT_C - TEMP_HYSTERESIS_C.
#include <Arduino.h>
#include <DHT.h>
#include <ESP32Servo.h>
#include "config.h"
#include "net.h"

static DHT dht(PIN_DHT, DHT11);
static Servo window;

static bool autoMode = true;
static bool windowOpen = false;
static bool overheat = false;
static unsigned long lastRead = 0;

static void setWindow(bool open) {
  window.write(open ? WINDOW_ANGLE_OPEN : WINDOW_ANGLE_CLOSED);
  windowOpen = open;
  net::publish("window/state", open ? "open" : "closed", true);
}

static void setMode(bool automatic) {
  autoMode = automatic;
  net::publish("window/mode", automatic ? "auto" : "manual", true);
}

static void onCommand(const String& sub, const String& payload) {
  if (sub != "window/set") return;
  if (payload == "auto") { setMode(true); setWindow(overheat); }
  else if (payload == "open")  { setMode(false); setWindow(true); }
  else if (payload == "close") { setMode(false); setWindow(false); }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED_ALERT, OUTPUT);
  dht.begin();

  window.setPeriodHertz(50);
  window.attach(PIN_WINDOW_SERVO, 500, 2400);

  net::begin("kitchen", onCommand);
  setWindow(false);
  setMode(true);
  net::publish("alert", "normal", true);
  Serial.println("Kitchen node ready.");
}

void loop() {
  net::loop();

  // Blink the alert LED while overheating
  digitalWrite(PIN_LED_ALERT, overheat && (millis() / 300) % 2);

  if (millis() - lastRead < CLIMATE_PERIOD_MS) return;
  lastRead = millis();

  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(t) || isnan(h)) {
    Serial.println("DHT11 read failed");
    return;
  }
  net::publish("climate", "{\"t\":" + String(t, 1) + ",\"h\":" + String(h, 1) + "}");

  bool hot = overheat ? t > TEMP_ALERT_C - TEMP_HYSTERESIS_C : t > TEMP_ALERT_C;
  if (hot != overheat) {
    overheat = hot;
    net::publish("alert", hot ? "overheat" : "normal", true);
    if (autoMode) setWindow(hot);
  }
}
