#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include "globals.hpp"
#include "util.hpp"
#include "display.hpp"

/*
Called when we get assigned an IP at the network we're connecting to
*/
void on_wifi_connect(WiFiEvent_t event, WiFiEventInfo_t info) {
  digitalWrite(WIFI_CONNECTED_LED_PIN, HIGH);
  stopDisconnectBeep = false;
  play_tone(2);

  prefs.begin("config");

  Serial.print(F("[INFO ] Wi-Fi STA Connected to "));
  Serial.print(prefs.getString("ssid", "KEY NOT FOUND"));
  Serial.print(F("! IP: "));
  Serial.print(WiFi.localIP());
  Serial.print(F(" | MAC: "));
  Serial.println(WiFi.macAddress());

  print_display_idle_info();
  prefs.end();
}

/*
Called when the wifi connection is lost or reset
*/
void on_wifi_disconnect(WiFiEvent_t event, WiFiEventInfo_t info) {
  digitalWrite(WIFI_CONNECTED_LED_PIN, LOW);
  if (!stopDisconnectBeep) {
    stopDisconnectBeep = true;
    play_tone(3);
  }

  Serial.println(F("[INFO ] Wi-Fi STA Disconnected!"));

  print_display_idle_info();
}
