// Living room node — presence-based lighting.
//
// Publishes : home/living/motion        1 | 0
//             home/living/light/state   on | off         (retained)
//             home/living/light/mode    auto | manual    (retained)
// Listens   : home/living/light/set     on | off | auto
//   "on"/"off" switch to manual mode, "auto" gives control back to the PIR.
#include <Arduino.h>
#include "config.h"
#include "net.h"

static bool autoMode = true;
static bool lightOn = false;
static bool motion = false;
static unsigned long lastMotion = 0;

static void setLight(bool on) {
  if (on == lightOn) return;
  lightOn = on;
  digitalWrite(PIN_LIGHT, on ? HIGH : LOW);
  net::publish("light/state", on ? "on" : "off", true);
}

static void setMode(bool automatic) {
  autoMode = automatic;
  net::publish("light/mode", automatic ? "auto" : "manual", true);
}

static void onCommand(const String& sub, const String& payload) {
  if (sub != "light/set") return;
  if (payload == "auto") setMode(true);
  else if (payload == "on")  { setMode(false); setLight(true); }
  else if (payload == "off") { setMode(false); setLight(false); }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_LIGHT, OUTPUT);
  digitalWrite(PIN_LIGHT, LOW);

  net::begin("living", onCommand);
  net::publish("light/state", "off", true);
  setMode(true);
  Serial.println("Living room node ready (PIR warm-up ~30 s).");
}

void loop() {
  net::loop();

  bool m = digitalRead(PIN_PIR) == HIGH;
  if (m) lastMotion = millis();
  if (m != motion) {
    motion = m;
    net::publish("motion", m ? "1" : "0");
  }

  if (autoMode) setLight(millis() - lastMotion < LIGHT_HOLD_MS && lastMotion != 0);
}
