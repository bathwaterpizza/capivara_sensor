#pragma once

#include <Arduino.h>
#include <rdm6300.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include <ESPAsyncWebSrv.h>
#include <Preferences.h>

extern bool stopDisconnectBeep;
extern Rdm6300 rdm6300;
extern Adafruit_SSD1306 display;
extern HTTPClient http;
extern AsyncWebServer server;
extern Preferences prefs;
