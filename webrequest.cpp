#include "webrequest.hpp"
#include <Preferences.h>  // data storage on flash memory
#include <WiFi.h>         // wifi
#include <HTTPClient.h>   // http
#include <mbedtls/md.h>   // esp32 hashing
#include "display.hpp"
#include "util.hpp"
#include "secrets.hpp"
#include "globals.hpp"

/*
This task encapsulates the process of sending an http request, receiving a response and updating the display
Requires more than 1k bytes of stack memory to work properly, been using 10k
Params is a dynamic array of two strings, hashed_tag_id and tag_id
Here were send "tag_id" and "classroom_id" to the api, and expect to receive the student's first name or empty
*/
void task_http_post(void* params) {
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