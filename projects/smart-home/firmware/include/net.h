// Wi-Fi + MQTT (TLS) helper shared by the 3 nodes.
// Each node publishes under home/<node>/... and keeps a retained
// home/<node>/status = online/offline (Last Will).
#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "config.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "Copy include/secrets.example.h to include/secrets.h and fill in your Wi-Fi and EMQX credentials."
#endif

namespace net {

using Handler = void (*)(const String& subtopic, const String& payload);

inline WiFiClientSecure tls;
inline PubSubClient mqtt(tls);
inline String node;
inline Handler handler = nullptr;
inline unsigned long lastAttempt = 0;

inline String topic(const char* sub) { return String(TOPIC_ROOT) + "/" + node + "/" + sub; }

inline void publish(const char* sub, const String& payload, bool retained = false) {
  mqtt.publish(topic(sub).c_str(), payload.c_str(), retained);
  Serial.printf("[MQTT] -> %s = %s\n", topic(sub).c_str(), payload.c_str());
}

inline void onMessage(char* t, byte* data, unsigned int len) {
  String full(t), payload;
  payload.reserve(len);
  for (unsigned int i = 0; i < len; i++) payload += (char)data[i];
  payload.trim();
  String prefix = String(TOPIC_ROOT) + "/" + node + "/";
  String sub = full.startsWith(prefix) ? full.substring(prefix.length()) : full;
  Serial.printf("[MQTT] <- %s = %s\n", full.c_str(), payload.c_str());
  if (handler) handler(sub, payload);
}

inline void connectWifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.printf("[WiFi] connecting to %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(300);
    Serial.print('.');
  }
  Serial.println(WiFi.status() == WL_CONNECTED ? " OK " + WiFi.localIP().toString() : " failed");
}

// Subscribes to home/<node>/+/set after each (re)connection.
inline bool connectMqtt() {
  if (mqtt.connected()) return true;
  if (millis() - lastAttempt < 3000) return false;  // non-blocking retry
  lastAttempt = millis();
  connectWifi();
  if (WiFi.status() != WL_CONNECTED) return false;

  String clientId = "smarthome-" + node + "-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  String will = topic("status");
  Serial.printf("[MQTT] connecting to %s:%d ... ", MQTT_HOST, MQTT_PORT);
  if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD, will.c_str(), 1, true, "offline")) {
    Serial.println("OK");
    mqtt.publish(will.c_str(), "online", true);
    mqtt.subscribe(topic("+/set").c_str(), 1);
    digitalWrite(PIN_STATUS_LED, HIGH);
    return true;
  }
  Serial.printf("failed (state %d)\n", mqtt.state());
  digitalWrite(PIN_STATUS_LED, LOW);
  return false;
}

inline void begin(const char* nodeName, Handler h) {
  node = nodeName;
  handler = h;
  pinMode(PIN_STATUS_LED, OUTPUT);
#ifdef MQTT_CA_CERT
  tls.setCACert(MQTT_CA_CERT);
#else
  tls.setInsecure();
#endif
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMessage);
  mqtt.setBufferSize(512);
  connectWifi();
  connectMqtt();
}

inline void loop() {
  if (!mqtt.connected()) {
    digitalWrite(PIN_STATUS_LED, LOW);
    connectMqtt();
  }
  mqtt.loop();
}

}  // namespace net
