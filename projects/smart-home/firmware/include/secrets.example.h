// Copy this file to secrets.h and fill in your own values.
// secrets.h is ignored by git: never commit real credentials.
#pragma once

// Wi-Fi
#define WIFI_SSID     "your-wifi-name"
#define WIFI_PASSWORD "your-wifi-password"

// EMQX Cloud deployment (Overview page of the deployment)
#define MQTT_HOST     "xxxxxxxx.ala.eu-central-1.emqxsl.com"
#define MQTT_PORT     8883            // TLS port
#define MQTT_USER     "smarthome"     // created in Access Control > Authentication
#define MQTT_PASSWORD "change-me"

// CA certificate of the deployment (download it from the EMQX Cloud console).
// If MQTT_CA_CERT is not defined, the connection is still encrypted but the
// server certificate is NOT verified — acceptable for a demo, not for real use.
// #define MQTT_CA_CERT R"EOF(
// -----BEGIN CERTIFICATE-----
// ...
// -----END CERTIFICATE-----
// )EOF"
