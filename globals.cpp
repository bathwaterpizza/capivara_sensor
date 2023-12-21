#include "globals.hpp"

bool stopDisconnectBeep = false;  // this is not a config, do not change
Rdm6300 rdm6300;
Adafruit_SSD1306 display(128, 64, &Wire, -1);
HTTPClient http;
AsyncWebServer server(80);
Preferences prefs;
