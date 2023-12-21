// Note: Serial1 is free, can be assigned to GPIO 2, 4
#include <rdm6300.h>         // rfid reader
#include <WiFi.h>            // wifi
#include <HTTPClient.h>      // http
#include <AsyncTCP.h>        // dependency for web server
#include <Preferences.h>     // data storage on flash memory
#include <ESPAsyncWebSrv.h>  // esp32 web server
#include <esp_wpa2.h>        // esp32 WPA2-E support
#include <mbedtls/md.h>      // esp32 hashing
#include <ESP.h>             // esp32 software reboot

// display libs
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// constants
#include "secrets.hpp"
#include "util.hpp"
#include "globals.hpp"
#include "events.hpp"
#include "display.hpp"

// config
//#define DEBUG_IGNORE_WIFI       // useful for testing
//#define OVERWRITE_ON_BOOT       // overwrite preferences stored on flash with hardcoded values

/*
This task encapsulates the process of sending an http request, receiving a response and updating the display
Requires more than 1k bytes of stack memory to work properly, been using 10k
Params is a dynamic array of two strings, hashed_tag_id and tag_id
Here were send "tag_id" and "classroom_id" to the api, and expect to receive the student's first name or empty
*/
void task_http_post(void* params) {
  void print_display_idle_info(bool onTagRead = false);

  String* paramStr = static_cast<String*>(params);
  Serial.print(F("[DEBUG] Hash received in task: "));
  Serial.println(paramStr[0]);

  http.begin(POST_ADDRESS);
  http.addHeader("Content-Type", "application/json");

  prefs.begin("config");

  int responseCode = http.POST(  // this call is blocking
    "{\"tag_id\":\"" + paramStr[0] + "\",\"classroom_id\":\"" + prefs.getString("classroom", "KEY NOT FOUND") + "\"}");
  String responseData = http.getString();

  prefs.end();
  http.end();

  Serial.print(F("[DEBUG] Response code: "));
  Serial.println(responseCode);
  Serial.print(F("[DEBUG] Response data: "));
  Serial.println(responseData);

  if (responseCode > 0) { 
    print_display_welcome(paramStr[1], responseData);
  } else {  // error
    print_display_http_error(paramStr[1]);
  }
  vTaskDelay(pdMS_TO_TICKS(1500));
  print_display_idle_info();

  delete[] paramStr;
  vTaskDelete(NULL);
}

/*
Takes the array of bytes output from hashing and converts it to a hexadecimal string,
which is the usual text representation, to send over wifi
*/
String hash_bytes_to_hex_str(byte* hash) {
  String hexStr = "";

  for (int i = 0; i < 32; i++) {
    if (hash[i] < 0x10)
      hexStr += '0';

    hexStr += String(hash[i], HEX);
  }

  return hexStr;
}

/*
Hashes the tag parameter and sends it over the wifi that is currently connected
*/
void send_http_post(String tag) {
  byte hashResult[32];
  mbedtls_md_context_t hashConfig;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

  mbedtls_md_init(&hashConfig);
  mbedtls_md_setup(&hashConfig, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&hashConfig);
  mbedtls_md_update(&hashConfig, (const unsigned char*)tag.c_str(), (size_t)tag.length());
  mbedtls_md_finish(&hashConfig, hashResult);
  mbedtls_md_free(&hashConfig);

  // send hashed tag
  Serial.println(F("[INFO ] Initializing HTTP POST task"));

  String* tagStrParam = new String[2]{ hash_bytes_to_hex_str(hashResult), tag };
  xTaskCreate(
    task_http_post,
    "HTTP POST task",
    10000,
    tagStrParam,
    1,
    NULL);
}

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
  Serial2.begin(RDM6300_BAUDRATE, SERIAL_8N1, 19, 22);  // RX/TX 2
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