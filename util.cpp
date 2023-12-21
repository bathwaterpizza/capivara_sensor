#include "util.hpp"

/*
This task blinks the rfid reading indicator LED
It takes one dynamically allocated int as a parameter, which is the blink duration, and frees it
*/
void task_blink_read_led(void* params) {
  int* param = static_cast<int*>(params);

  digitalWrite(RFID_READ_LED_PIN, HIGH);

  vTaskDelay(pdMS_TO_TICKS(*param));

  digitalWrite(RFID_READ_LED_PIN, LOW);

  delete param;
  vTaskDelete(NULL);
}

/*
Creates a priority 2 task that blinks the green reading status LED for (blinkTime)ms
blinkTime is optional and defaults to 1 second
*/
void blink_read_led(int blinkTime) {
  int* param = new int{ blinkTime };

  xTaskCreate(
    task_blink_read_led,
    "Read LED task",
    1000,
    param,
    2,
    NULL);
}

/*
This task plays a sound using the buzzer
It takes a dynamically allocated array of 3 ints as a parameter, which specifies the amount of times to repeat,
the beep time and the sleep time between beeps, in that order. It also frees the array.
*/
void task_play_tone(void* params) {
  int* array = static_cast<int*>(params);

  for (int i = 0; i < array[0]; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(array[1]));

    digitalWrite(BUZZER_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(array[2]));
  }

  delete[] array;
  vTaskDelete(NULL);
}

/*
Creates a priority 1 task that loops (count) times. Each loop beeps for (buzzTime)ms and sleeps for (sleepTime)ms.
Time arguments are optional.
*/
void play_tone(int count, int buzzTime, int sleepTime) {
  int* params = new int[3]{ count, buzzTime, sleepTime };

  xTaskCreate(
    task_play_tone,
    "Buzzer task",
    1000,
    params,
    1,
    NULL);
}

/*
Synchronous version of play_tone
*/
void play_tone_blocking(int count, int buzzTime, int sleepTime) {
  for (int i = 0; i < count; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(buzzTime);

    digitalWrite(BUZZER_PIN, LOW);
    delay(sleepTime);
  }
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
