#include <rdm6300.h>     // rfid
#include <WiFi.h>        // wifi
#include <HTTPClient.h>  // wifi
#include "esp_wpa2.h"    // WPA2-E support
#include "mbedtls/md.h"  // hashing

// reassigned UART 1 to GPIO 2, 4
#include <HardwareSerial.h>
#include <DFPlayerMini_Fast.h>

// display libs
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// network config
#include "secrets.hpp"

#define SSD1306_NO_SPLASH
//#define DEBUG_IGNORE_WIFI //dont use wifi

Rdm6300 rdm6300;
HTTPClient http;
DFPlayerMini_Fast player;
Adafruit_SSD1306 display(128, 64, &Wire, -1);

String hashBytesToHexString(byte* hash) {
  String hexStr = "";

  for (int i = 0; i < 32; i++) {
    if (hash[i] < 0x10)
      hexStr += '0';

    hexStr += String(hash[i], HEX);
  }

  return hexStr;
}

void print_display_text(String text, int start_y, int size) {
  display.clearDisplay();
  display.setTextSize(size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, start_y);  // Start at top-left corner
  display.println(text.c_str());
  display.display();
}

int sendPOST(String tag) {
  // # (hash tag.. lmao)
  Serial.println(F("[INFO ] Hashing"));

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
  Serial.println(F("[INFO ] Sending HTTP POST request"));

  http.begin(POST_ADDRESS);
  http.addHeader("Content-Type", "application/json");

  int responseCode = http.POST("{\"tag_id\":\"" + hashBytesToHexString(hashResult) + "\"}");

  http.end();

  return responseCode;
}

void setup() {
  Serial.begin(115200);             // RX/TX 1 on the board, which actually maps to Serial 0
  Serial2.begin(RDM6300_BAUDRATE);  // RX/TX 2
  rdm6300.begin(&Serial2);

  Serial1.begin(9600, SERIAL_8N1, 4, 2);  // ESP32: RX1 = D4, TX1 = D2
  player.begin(Serial1, false);
  player.volume(30);  // max volume

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  delay(1000);  //wait for serial monitor
#ifndef DEBUG_IGNORE_WIFI
  WiFi.disconnect(true);  //clean up previous connections

  Serial.print(F("[INFO ] Connecting to "));
  Serial.println(SSID);

  //WiFi.begin(SSID, WPA2_AUTH_PEAP, EAP_ANONYMOUS_IDENTITY, EAP_IDENTITY, PASSWORD, test_root_ca); // with cert
  if (WPA2_ENTERPRISE) {
    WiFi.begin(SSID, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, PASSWORD);  // without cert, wpa2-e
  } else {
    WiFi.begin(SSID, PASSWORD);  // without cert, wpa2-psk
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }

  Serial.print(F("\n[INFO ] Connected! IP: "));
  Serial.print(WiFi.localIP());
  Serial.print(F(" | MAC: "));
  Serial.println(WiFi.macAddress());
#endif
}

void loop() {
  if (rdm6300.get_new_tag_id()) {  // true when a new tag shows up in the buffer
    uint32_t tag = rdm6300.get_tag_id();
    String tagStr = String(tag, HEX);
    tagStr.toUpperCase();
    int response;

    Serial.print(F("[EVENT] Tag received: "));
    Serial.print(tag);
    Serial.print(F(" (0x"));
    Serial.print(tagStr);
    Serial.println(F(")"));
    print_display_text(tagStr, 0, 3);

    player.playNext();  //sound

#ifndef DEBUG_IGNORE_WIFI
    response = sendPOST(tagStr);  // loop gets halted here, could be in a different task

    Serial.print(F("[EVENT] Got response: "));
    Serial.println(response);
#endif
  }

#ifndef DEBUG_IGNORE_WIFI
  if (WiFi.status() != WL_CONNECTED)
    WiFi.reconnect();
#endif
}