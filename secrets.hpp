/*
Variables for network configuration
Their definitions should be hidden from git

You must define OVERWRITE_ON_BOOT in order to save these to flash preferences
*/

#pragma once

// this is used to provide the wifi options available on the config webpage
struct wifi_creds_struct {
  const char* ssid;
  const char* password;
  const char* eap;
  const bool enterprise;
};
typedef struct wifi_creds_struct WiFiCredentials;

extern const WiFiCredentials WIFI_LIST[];

extern const char* CLASSROOM_ID;    // classroom name, e.g. "L540"
extern const char* POST_ADDRESS;    // URL for the HTTP POST request
extern const char* AP_SSID;         // name of wifi access point for configuration
extern const char* AP_PASSWORD;     // password
extern const bool WPA2_ENTERPRISE;  // true if connecting to a WPA2-E network

extern const char* EAP_ANONYMOUS_IDENTITY;
extern const char* EAP_IDENTITY;
extern const char* EAP_USERNAME;
extern const char* SSID;
extern const char* PASSWORD;
