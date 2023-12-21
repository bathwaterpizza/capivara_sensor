#include "events.hpp"
#include <Preferences.h>  // flash storage
#include <ESP.h>          // esp32 software reboot
#include "globals.hpp"
#include "util.hpp"
#include "display.hpp"
#include "secrets.hpp"

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


/*
Called when a client requests the setwifi route from our web server AP
*/
void on_route_setwifi(AsyncWebServerRequest *request) {
  String dataReceived;

  if (request->hasParam("wifi", true)) {
    play_tone_blocking(2, 50, 150);

    dataReceived = request->getParam("wifi", true)->value();
    int index = dataReceived.toInt();
    auto credentials = WIFI_LIST[index];
    Serial.print("[DEBUG] Got Wi-Fi choice from webpage: ");
    Serial.println(dataReceived);

    prefs.begin("config");

    prefs.putString("ssid", credentials.ssid);
    prefs.putString("password", credentials.password);
    prefs.putString("eap", credentials.eap);
    prefs.putBool("isEnterprise", credentials.enterprise);

    prefs.end();
  }

  request->send(200);
  ESP.restart();
}

/*
Called when a client requests the setclassroom route from our web server AP
*/
void on_route_setclassroom(AsyncWebServerRequest *request) {
  String dataReceived;

  if (request->hasParam("classroom_id", true)) {
    play_tone_blocking(2, 50, 150);

    dataReceived = request->getParam("classroom_id", true)->value();
    Serial.print("[DEBUG] Got CLASSROOM_ID choice from webpage: ");
    Serial.println(dataReceived);

    prefs.begin("config");

    prefs.putString("classroom", dataReceived);

    prefs.end();
  }

  request->send(200);
  ESP.restart();
}
