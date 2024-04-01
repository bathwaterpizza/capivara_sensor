#include "events.hpp"
#include <Preferences.h>  // flash storage
#include <Esp.h>          // esp32 software reboot
#include "globals.hpp"
#include "util.hpp"
#include "display.hpp"
#include "secrets.hpp"

/*
Called when we get assigned an IP at the network we're connecting to
*/
void on_wifi_connect(WiFiEvent_t event, WiFiEventInfo_t info) {
  // digitalWrite(WIFI_CONNECTED_LED_PIN, HIGH);
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
  // digitalWrite(WIFI_CONNECTED_LED_PIN, LOW);
  if (!stopDisconnectBeep) {
    stopDisconnectBeep = true;
    play_tone(3);
  }

  Serial.println(F("[INFO ] Wi-Fi STA Disconnected!"));

  print_display_idle_info();
}

/*
Called when a client submits the device settings form in our web server AP
*/
void on_route_setcfg(AsyncWebServerRequest *request) {
  String wifiDataReceived, classroomDataReceived;

  if (request->hasParam("wifi", true) && request->hasParam("classroom_id", true)) {
    play_tone_blocking(2, 50, 150);

    wifiDataReceived = request->getParam("wifi", true)->value();
    int index = wifiDataReceived.toInt();
    auto credentials = WIFI_LIST[index];
    Serial.print("[DEBUG] Got Wi-Fi choice from webpage: ");
    Serial.println(wifiDataReceived);

    classroomDataReceived = request->getParam("classroom_id", true)->value();
    Serial.print("[DEBUG] Got CLASSROOM_ID choice from webpage: ");
    Serial.println(classroomDataReceived);

    prefs.begin("config");

    prefs.putString("ssid", credentials.ssid);
    prefs.putString("password", credentials.password);
    prefs.putString("eap", credentials.eap);
    prefs.putBool("isEnterprise", credentials.enterprise);

    prefs.putString("classroom", classroomDataReceived);

    prefs.end();
  }

  request->send(200);
  ESP.restart();
}
