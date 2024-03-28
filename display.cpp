#include "display.hpp"
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include "globals.hpp"

/*
Wrapping some functionality from the display library to print text to our display,
given a size and the starting height
*/
void print_display_text(String text, int start_y, int size) {
  display.clearDisplay();

  display.setTextSize(size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, start_y);
  display.println(text);

  display.display();
}

/*
Displays the tag ID and a welcome message, which may include the student's name if there was a response,
Called when the http request succeeds
*/
void print_display_welcome(String tag, String firstName) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Tag: " + tag);

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 20);
  display.println("Oi,");

  display.setCursor(0, 40);
  if (!firstName.isEmpty()) {
    display.println(firstName);
  } else {
    display.println("Aluno");
  }

  display.display();
}

/*
Displays the tag ID and an error message,
Called when the http request fails
*/
void print_display_http_error(String tag) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Tag: " + tag);

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 30);
  display.println("Erro HTTP!");

  display.display();
}

/*
Prints standard information to the display when idle, such as:
- Wifi connection status
- Current classroom_id
- Reader status
*/
void print_display_idle_info(bool onTagRead) {
  bool isConnected = WiFi.isConnected();

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  if (isConnected) {
    display.println(WiFi.SSID() + " (" + WiFi.RSSI() + " dBm)");
  } else {
    display.println(F("Wi-Fi sem sinal!"));
  }

  prefs.begin("config");

  display.setCursor(0, 9);
  display.println("Turma " + prefs.getString("classroom", "KEY NOT FOUND"));

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  if (onTagRead) {
    display.setCursor(11, 33);
    display.println(F("AGUARDE.."));
  } else {
    display.setCursor(31, 25);
    display.println(F("LEITOR"));

    if (isConnected) {
      display.setCursor(37, 45);
      display.println(F("ATIVO"));
    } else {
      display.setCursor(25, 45);
      display.println(F("OFFLINE"));
    }
  }

  display.display();
  prefs.end();
}

/*
Prints the project name to the display while the device is initializing
*/
void print_display_init_screen() {
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(17, 24);
  display.println(F("CAPIVARA"));
  display.drawRoundRect(10, 15, 108, 34, 12, SSD1306_WHITE);

  display.display();
}
