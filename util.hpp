#pragma once

#define RFID_READ_LED_PIN 5        // green led
#define WIFI_CONNECTED_LED_PIN 13  // blue led
#define BUZZER_PIN 12              // active buzzer

#include <Arduino.h>

void blink_read_led(int blinkTime = 1000);
void play_tone(int count, int buzzTime = 200, int sleepTime = 200);
void play_tone_blocking(int count, int buzzTime = 200, int sleepTime = 200);
String hash_bytes_to_hex_str(byte* hash);
