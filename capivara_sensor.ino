#include <rdm6300.h>            // rfid reader
#include <WiFi.h>               // wifi
#include <HTTPClient.h>         // wifi
#include "esp_wpa2.h"           // esp32 WPA2-E support
#include "mbedtls/md.h"         // esp32 hashing
#include <HardwareSerial.h>     // UART serial. note: reassigned Serial1 to GPIO 2, 4
#include <DFPlayerMini_Fast.h>  // amplifier
// trying without this first
//#include <FreeRTOS.h>          // scheduler
//#include <task.h>              // tasks

// display libs
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// constants
#include "secrets.hpp"
#define RFID_READ_LED_PIN 5
#define WIFI_CONNECTED_LED_PIN 13
#define BUZZER_PIN 12

// config
#define SSD1306_NO_SPLASH  // disable display startup art
//#define DEBUG_IGNORE_WIFI  // useful for testing

// globals
Rdm6300 rdm6300;
HTTPClient http;
DFPlayerMini_Fast player;
Adafruit_SSD1306 display(128, 64, &Wire, -1);

/*
This task blinks the rfid reading indicator LED for 1 second
*/
void task_blink_read_led(void* params) {
  digitalWrite(RFID_READ_LED_PIN, HIGH);

  vTaskDelay(pdMS_TO_TICKS(1000));  // local delay

  digitalWrite(RFID_READ_LED_PIN, LOW);

  vTaskDelete(NULL);  // delete current task, this might be unnecessary
}

void task_play_tone(void* params) {
  digitalWrite(BUZZER_PIN, HIGH);

  vTaskDelay(pdMS_TO_TICKS(200));
  
  digitalWrite(BUZZER_PIN, LOW);

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
Wrapping some functionality from the display library to print text to our display,
given a size and the starting height
*/
void print_display_text(String text, int start_y, int size) {
  display.clearDisplay();
  display.setTextSize(size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, start_y);  // Start at top-left corner
  display.println(text.c_str());
  display.display();
}

int send_http_post(String tag) {
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

  int responseCode = http.POST("{\"tag_id\":\"" + hash_bytes_to_hex_str(hashResult) + "\"}");

  http.end();

  return responseCode;
}

void on_wifi_connect() {
  digitalWrite(WIFI_CONNECTED_LED_PIN, HIGH);

  Serial.print(F("\n[INFO ] Wi-Fi Connected! IP: "));
  Serial.print(WiFi.localIP());
  Serial.print(F(" | MAC: "));
  Serial.println(WiFi.macAddress());
}

void on_wifi_disconnect() {
  digitalWrite(WIFI_CONNECTED_LED_PIN, LOW);

  Serial.println(F("\n[INFO ] Wi-Fi Disconnected!"));

  // try reconnect
}

void setup() {
  // pin setup
  pinMode(RFID_READ_LED_PIN, OUTPUT);
  pinMode(WIFI_CONNECTED_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(RFID_READ_LED_PIN, LOW);
  digitalWrite(WIFI_CONNECTED_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // serial setup
  Serial.begin(115200);             // RX/TX 1 on the board, which actually maps to Serial 0
  Serial2.begin(RDM6300_BAUDRATE);  // RX/TX 2
  rdm6300.begin(&Serial2);

  // note we are reassigning UART 1 to RX1 = D4, TX1 = D2 on the ESP32
  Serial1.begin(9600, SERIAL_8N1, 4, 2);
  player.begin(Serial1, false);
  player.volume(30);  // max volume

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // this I2C address is specific to our display module
  display.clearDisplay();

  delay(1000);  //wait for serial monitor
#ifndef DEBUG_IGNORE_WIFI
  // wifi setup
  WiFi.disconnect(true);  //clean up previous connections

  Serial.print(F("[INFO ] Connecting to "));
  Serial.println(SSID);

  // registering callbacks
  WiFi.onEvent(on_wifi_connect, SYSTEM_EVENT_STA_CONNECTED);
  WiFi.onEvent(on_wifi_disconnect, SYSTEM_EVENT_STA_DISCONNECTED);

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
#endif
}

void loop() {
  if (rdm6300.get_new_tag_id()) {  // true when a new tag shows up in the uart buffer
    xTaskCreate( // create a task to blink the read LED
      task_blink_read_led,
      "Read LED task",
      1000,
      NULL,
      2,
      NULL
    );
    xTaskCreate( // create a task to play a tone using our buzzer
      task_play_tone,
      "Buzzer task",
      1000,
      NULL,
      1,
      NULL
    );

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

    player.playNext();  //play sound

#ifndef DEBUG_IGNORE_WIFI
    response = send_http_post(tagStr);  // loop gets halted here, could be in a different task

    Serial.print(F("[EVENT] Got response: "));
    Serial.println(response);
#endif
  }

#ifndef DEBUG_IGNORE_WIFI
  if (WiFi.status() != WL_CONNECTED) // not necessary after we implement events, and reconnect inside the disconnect event, or figure out the auto reconnect
  {
    WiFi.reconnect();
  }
#endif
}