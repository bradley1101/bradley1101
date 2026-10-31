// Entrance node — RFID access control, motorised door, visitor detection.
//
// Publishes : home/entrance/access        {"uid":"A1B2C3D4","granted":true}
//             home/entrance/door/state    open | closed           (retained)
//             home/entrance/visitor       present | absent
// Listens   : home/entrance/door/set      open | close
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include "config.h"
#include "net.h"

static MFRC522 rfid(PIN_RFID_SS, PIN_RFID_RST);
static Servo door;
static const char* authorized[] = AUTHORIZED_UIDS;

static bool doorOpen = false;
static unsigned long doorOpenedAt = 0;
static bool autoClose = false;
static bool visitor = false;
static unsigned long lastDistanceRead = 0;

static void setDoor(bool open, bool closeLater) {
  door.write(open ? DOOR_ANGLE_OPEN : DOOR_ANGLE_CLOSED);
  doorOpen = open;
  autoClose = open && closeLater;
  doorOpenedAt = millis();
  net::publish("door/state", open ? "open" : "closed", true);
}

static void flash(int pin) {
  digitalWrite(pin, HIGH);
  delay(800);
  digitalWrite(pin, LOW);
}

static String readUid() {
  String uid;
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += '0';
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

static bool isAuthorized(const String& uid) {
  for (const char* a : authorized)
    if (uid.equalsIgnoreCase(a)) return true;
  return false;
}

static float readDistanceCm() {
  digitalWrite(PIN_US_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_US_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_US_TRIG, LOW);
  unsigned long us = pulseIn(PIN_US_ECHO, HIGH, 25000);  // ~4 m max
  return us == 0 ? -1 : us / 58.0f;
}

static void onCommand(const String& sub, const String& payload) {
  if (sub == "door/set") {
    if (payload == "open") setDoor(true, false);     // stays open until "close"
    else if (payload == "close") setDoor(false, false);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED_GRANTED, OUTPUT);
  pinMode(PIN_LED_DENIED, OUTPUT);
  pinMode(PIN_US_TRIG, OUTPUT);
  pinMode(PIN_US_ECHO, INPUT);

  door.setPeriodHertz(50);
  door.attach(PIN_DOOR_SERVO, 500, 2400);

  SPI.begin();
  rfid.PCD_Init();

  net::begin("entrance", onCommand);
  setDoor(false, false);
  Serial.println("Entrance node ready — present a badge.");
}

void loop() {
  net::loop();

  // 1. Badge
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String uid = readUid();
    bool ok = isAuthorized(uid);
    Serial.printf("Badge %s -> %s\n", uid.c_str(), ok ? "GRANTED" : "DENIED");
    net::publish("access", "{\"uid\":\"" + uid + "\",\"granted\":" + (ok ? "true" : "false") + "}");
    if (ok) setDoor(true, true);
    flash(ok ? PIN_LED_GRANTED : PIN_LED_DENIED);
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  // 2. Automatic closing after a badge
  if (doorOpen && autoClose && millis() - doorOpenedAt > DOOR_OPEN_MS) setDoor(false, false);

  // 3. Visitor in front of the door
  if (millis() - lastDistanceRead > 500) {
    lastDistanceRead = millis();
    float d = readDistanceCm();
    bool present = d > 0 && d < VISITOR_CM;
    if (present != visitor) {
      visitor = present;
      net::publish("visitor", present ? "present" : "absent");
    }
  }
}
