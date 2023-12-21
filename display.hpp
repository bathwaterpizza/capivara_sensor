#pragma once

#define SSD1306_NO_SPLASH         // disable display startup art
#define DISPLAY_I2C_ADDRESS 0x3C  // this address is specific to our display

#include <Arduino.h>

void print_display_text(String text, int start_y, int size);
void print_display_welcome(String tag, String firstName);
void print_display_http_error(String tag);
void print_display_idle_info(bool onTagRead = false);
void print_display_init_screen();
