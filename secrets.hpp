/*
Variables for network configuration
Their definitions should be hidden from git
*/

#ifndef _NETWORK_SECRETS
#define _NETWORK_SECRETS

extern const char* POST_ADDRESS;
extern const bool WPA2_ENTERPRISE;

extern const char* EAP_ANONYMOUS_IDENTITY;
extern const char* EAP_IDENTITY;
extern const char* EAP_USERNAME;
extern const char* SSID;
extern const char* PASSWORD;

#endif