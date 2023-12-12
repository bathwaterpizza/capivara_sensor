/*
Variables for network configuration
Their definitions should be hidden from git
*/

#ifndef _NETWORK_SECRETS
#define _NETWORK_SECRETS

extern const char* CLASSROOM_ID;   // classroom name, e.g. "L540"
extern const char* POST_ADDRESS;   // URL for the HTTP POST request
extern const char* AP_SSID;        // wifi ap ssid for configuration
extern const char* AP_PASSWORD;    // password
extern const bool WPA2_ENTERPRISE; // true if connecting to a WPA2-E network

extern const char* EAP_ANONYMOUS_IDENTITY;
extern const char* EAP_IDENTITY;
extern const char* EAP_USERNAME;
extern const char* SSID;
extern const char* PASSWORD;

#endif