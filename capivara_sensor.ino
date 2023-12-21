// config
#define SERIAL2_RX_PIN 19
#define SERIAL2_TX_PIN 22
//#define DEBUG_IGNORE_WIFI       // useful for testing
//#define OVERWRITE_ON_BOOT       // overwrite preferences stored on flash with hardcoded values

// Note: Serial1 is free, can be assigned to GPIO 2, 4
#include <WiFi.h>            // wifi
#include <esp_wpa2.h>        // esp32 WPA2-E support
#include "secrets.hpp"
#include "util.hpp"
#include "globals.hpp"
#include "events.hpp"
#include "display.hpp"
#include "webpage.hpp"
#include "webrequest.hpp"

void setup() {
  // pin setup
  pinMode(RFID_READ_LED_PIN, OUTPUT);
  pinMode(WIFI_CONNECTED_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(RFID_READ_LED_PIN, LOW);
  digitalWrite(WIFI_CONNECTED_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // serial debug setup
  Serial.begin(115200);            // RX/TX 1 on the board, which actually maps to Serial 0
  while (!Serial) { delay(100); }  // wait for serial to be ready

  delay(1000);  // wait a little bit more..

  // display setup
  display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_I2C_ADDRESS);
  print_display_init_screen();

  // serial tag reader setup
  //Serial2.begin(RDM6300_BAUDRATE);  // RX/TX 2
  Serial2.begin(RDM6300_BAUDRATE, SERIAL_8N1, SERIAL2_RX_PIN, SERIAL2_TX_PIN);  // RX/TX 2
  rdm6300.begin(&Serial2);

#ifdef OVERWRITE_ON_BOOT
  prefs.begin("config");

  prefs.putString("ssid", SSID);
  prefs.putString("password", PASSWORD);
  prefs.putString("eap", EAP_USERNAME);
  prefs.putString("classroom", CLASSROOM_ID);
  prefs.putBool("isEnterprise", WPA2_ENTERPRISE);

  prefs.end();
#endif

#ifndef DEBUG_IGNORE_WIFI
  // wifi setup
  WiFi.disconnect(true);  // clean up previous connections
  WiFi.persistent(false);
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(true);  // reconnect if connection lost

  // registering callbacks
  //WiFi.onEvent(on_wifi_connect, ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(on_wifi_connect, ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(on_wifi_disconnect, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  Serial.print(F("[INFO ] Setting up AP with SSID "));
  Serial.println(AP_SSID);

  WiFi.mode(WIFI_AP_STA);             // act as both client and hotspot
  WiFi.softAP(AP_SSID, AP_PASSWORD);  // hotspot settings
  IPAddress defaultIP(192, 168, 1, 1);
  IPAddress subnetMask(255, 255, 255, 0);
  WiFi.softAPConfig(defaultIP, defaultIP, subnetMask);

  Serial.print(F("[INFO ] AP IP is "));
  Serial.println(WiFi.softAPIP());

  // flash storage init
  prefs.begin("config");

  Serial.print(F("[INFO ] STA Connecting to "));
  Serial.println(prefs.getString("ssid", "KEY NOT FOUND"));

  if (prefs.getBool("isEnterprise")) {
    // without cert, wpa2-e
    WiFi.begin(prefs.getString("ssid"), WPA2_AUTH_PEAP, prefs.getString("eap"), prefs.getString("eap"), prefs.getString("password"));
  } else {
    // without cert, wpa2-psk
    WiFi.begin(prefs.getString("ssid"), prefs.getString("password"));
  }

  // save
  prefs.end();

  // AP local server setup
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", webpage);
  });
  server.on("/setwifi", HTTP_POST, on_route_setwifi);
  server.on("/setclassroom", HTTP_POST, on_route_setclassroom);

  server.begin();
#endif
}

void loop() {
  if (rdm6300.get_new_tag_id()) {  // true when a new tag shows up in the buffer
    blink_read_led();
    play_tone(1);

    uint32_t tag = rdm6300.get_tag_id();
    String tagStr = String(tag, HEX);
    tagStr.toUpperCase();

    Serial.print(F("[EVENT] Tag received: "));
    Serial.print(tag);
    Serial.print(F(" (0x"));
    Serial.print(tagStr);
    Serial.println(F(")"));

#ifndef DEBUG_IGNORE_WIFI
    if (WiFi.isConnected()) {
      print_display_idle_info(true);
      send_http_post(tagStr);
    } else {
      print_display_idle_info();
    }
#endif
  }
}