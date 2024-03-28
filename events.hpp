#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wpa2.h>        // esp32 WPA2-E support
#include <AsyncTCP.h>        // dependency for web server
#include <ESPAsyncWebSrv.h>  // esp32 web server

void on_wifi_connect(WiFiEvent_t event, WiFiEventInfo_t info);
void on_wifi_disconnect(WiFiEvent_t event, WiFiEventInfo_t info);

void on_route_setcfg(AsyncWebServerRequest *request);
