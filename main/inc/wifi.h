#ifndef __WIFI_H__
#define __WIFI_H__

#include "esp_event.h"

#define WIFI_AP_MODE_SSID               "UzUs"
#define WIFI_AP_MODE_PASSWORD           "123456789"
#define WIFI_AP_MODE_MAX_CONNECTION     5

#define WIFI_STATION_MODE_SSID         "이원준"
#define WIFI_STATION_MODE_PASSWORD     "19980406" 
#define CHANNEL_STATION_MODE      1
#define RETRY_MAXIMUM_COUNT       10

#define WIFI_CONNECTED_BIT        BIT0
#define WIFI_FAIL_BIT             BIT1

#define WIFI_SCAN_MAX_SIZE        20

static void WiFiAPModeEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData);
static void WiFiStationModeEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData);
void WiFiInit();
void WiFiInitializeSet();
void WiFiModeInit();
void WiFiNVSInit();
void GetHTTPServer();
void WiFiScan();
void WiFiCheckBits();
esp_err_t SetWiFiInformation(char* userName, char *wifi, char* password);
esp_err_t GetWiFiInformation();

typedef struct {
    unsigned char wifiSsid[32];
    unsigned char wifiPassword[64];
} WIFI_INFO_t;

#endif