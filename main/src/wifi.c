#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_spiffs.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "../inc/wifi.h"
#include "../inc/http.h"
#include "../inc/file.h"

static const char* TAG = "WIFI";

static esp_vfs_spiffs_conf_t spiffsConfig = {
    .base_path = "/spiffs",
    .partition_label = NULL,
    .max_files = 5,
    .format_if_mount_failed = true
};



static EventGroupHandle_t wifiEventGroupHandler;
static int wifiRetryCount = 0;

static char wifiId[32] = "asd";
static char wifiPassword[64] = "asd";

/********************************************************************************
* WiFiAPModeEventHandler
* Parameter 
*   arg : Event, Data 이외의 전달되는 Data
*   eventBase : Event Handler의 기본 ID
*   eventId : 호출되는 Event Handle ID
*   eventData : 호출되는 Event Handle의 Data
* 
* Description : AP 모드를 연결하려고 할 때 호출되는 Event Handler
********************************************************************************/

static void WiFiAPModeEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData)
{
    ESP_LOGI(TAG, "[WiFiAPModeEventHandler] Start");
    if(eventId == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) eventData;
        ESP_LOGI(TAG, "AID : %d", event->aid);
    }
    else if(eventId == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) eventData;
        ESP_LOGI(TAG, "AID : %d", event->aid);
    }
    ESP_LOGI(TAG, "[WiFiAPModeEventHandler] End");
}

/********************************************************************************
* WiFiStationModeEventHandler
* Parameter 
*   arg : Event, Data 이외의 전달되는 Data
*   eventBase : Event Handler의 기본 ID
*   eventId : 호출되는 Event Handle ID
*   eventData : 호출되는 Event Handle의 Data
* 
* Description : STA 모드를 연결하려고 할 때 호출되는 Event Handler
********************************************************************************/

static void WiFiStationModeEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData)
{
    ESP_LOGI(TAG, "[WiFiStationModeEventHandler] Start");

    if(eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if(eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED) {
        if(wifiRetryCount < RETRY_MAXIMUM_COUNT) {
            esp_wifi_connect();
            wifiRetryCount++;
            ESP_LOGI(TAG, "[WiFiStationModeEventHandler] WiFi Disconnected and Retry Count : [%d]", wifiRetryCount);
    }
    else {
        xEventGroupSetBits(wifiEventGroupHandler, WIFI_FAIL_BIT);
    }
    }
    else if(eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) eventData;
        wifiRetryCount = 0;
        xEventGroupSetBits(wifiEventGroupHandler, WIFI_CONNECTED_BIT);
    }
    else {
        ESP_LOGI(TAG, "[WiFiStationModeEventHandler] WiFi is Null");
    }

    ESP_LOGI(TAG, "[WiFiStationModeEventHandler] End");
}

/********************************************************************************
* WiFiNVSInit
* 
* Description : WiFi 연결을 위한 NVS 영역 재정의
********************************************************************************/

void WiFiNVSInit()
{
    ESP_LOGI(TAG, "[WiFiNVSInit] Start");

    esp_err_t nvs = nvs_flash_init();
    if((nvs == ESP_ERR_NVS_NO_FREE_PAGES) || (nvs == ESP_ERR_NVS_NEW_VERSION_FOUND)) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs = nvs_flash_init();
    }
    ESP_ERROR_CHECK(nvs);

    ESP_LOGI(TAG, "[WiFiNVSInit] End");
}

void WiFiInit()
{
    ESP_LOGI(TAG, "[WiFiInit] Start");
    GetWiFiInformation();
    WiFiInitializeSet();
    WiFiModeInit();
    WiFiScan();

    ESP_LOGI(TAG, "[WiFiInit] End");
}

/********************************************************************************
* WiFiNVSInit
* 
* Description : WiFi 기본 정보 및 Event Handler 등록
********************************************************************************/

void WiFiInitializeSet()
{
    ESP_LOGI(TAG, "[WiFiInitializeSet] Start");

    WiFiNVSInit();
    ESP_ERROR_CHECK(esp_netif_init());

    wifiEventGroupHandler = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_t* apNetIf = esp_netif_create_default_wifi_ap();
    assert(apNetIf);

    esp_netif_t* stationNetIf = esp_netif_create_default_wifi_sta();
    assert(stationNetIf);
 
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &WiFiStationModeEventHandler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WiFiAPModeEventHandler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "[WiFiInitializeSet] End");
}

/********************************************************************************
* WiFiNVSInit
* 
* Description : 정의된 Wifi 정보로 Wifi 연결 시도
********************************************************************************/

void WiFiModeInit()
{
    ESP_LOGI(TAG, "[WiFiModeInit] Start");

    wifi_config_t stationModeConfig = {};
    strcpy((char*)stationModeConfig.sta.ssid, wifiId);
    strcpy((char*)stationModeConfig.sta.password, wifiPassword);

    wifi_config_t apModeConfig = {
        .ap = {
            .ssid = WIFI_AP_MODE_SSID,
            .password = WIFI_AP_MODE_PASSWORD,
            .max_connection = WIFI_AP_MODE_MAX_CONNECTION,
            .ssid_len = strlen(WIFI_AP_MODE_SSID),
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .max_connection = WIFI_AP_MODE_MAX_CONNECTION,
        },
    };
    GetHTTPServer();
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &apModeConfig));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &stationModeConfig));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "[WiFiModeInit] AP Mode SSID : [%s], Password : [%s]", WIFI_AP_MODE_SSID, WIFI_AP_MODE_PASSWORD);
    
    ESP_ERROR_CHECK(esp_wifi_connect());

    ESP_LOGI(TAG, "[WiFiModeInit] End");
}

void GetHTTPServer()
{
    ESP_LOGI(TAG, "[GetHTTPServer] Start");
    //static httpd_handle_t  server = NULL;
    //ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &HTTPServerConnectHandler, &server));
    HTTPServerStart();
    ESP_LOGI(TAG, "[GetHTTPServer] End");
}

void WiFiScan()
{
    uint16_t wifiScanNumber = WIFI_SCAN_MAX_SIZE;
    wifi_ap_record_t wifiScanList[WIFI_SCAN_MAX_SIZE];
    uint16_t wifiScanCount = 0;
    memset(wifiScanList, 0, sizeof(wifiScanList));

    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&wifiScanNumber, wifiScanList));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&wifiScanCount));
    ESP_LOGI(TAG, "Find WiFi Count : %u", wifiScanCount);

    for (int i = 0; (i < WIFI_SCAN_MAX_SIZE) && (i < wifiScanCount); i++) {
        ESP_LOGI(TAG, "SSID \t\t%s", wifiScanList[i].ssid);
        ESP_LOGI(TAG, "RSSI \t\t%d", wifiScanList[i].rssi);
        ESP_LOGI(TAG, "Channel \t\t%d\n", wifiScanList[i].primary);
    }
}

/********************************************************************************
* SetWiFiInformation
* Parameter 
*   wifi : STA 모드에 사용될 WiFI
*   password : STA 모드에 사용될 Password
* 
* Description : AP 모드에서 입력 받은 Wifi 정보를 저장하는 함수
********************************************************************************/

esp_err_t SetWiFiInformation(char *userName, char *wifi, char* password)
{
    ESP_LOGI(TAG, "[SetWiFiInformation] Start");
    ESP_LOGI(TAG, "[SetWiFiInformation] wifi : [%s], password : [%s]", wifi, password);

    esp_err_t ret = esp_vfs_spiffs_register(&spiffsConfig);

    // if (ret != ESP_OK) {
    //     if (ret == ESP_FAIL) {
    //         ESP_LOGI(TAG, "Failed to mount or format filesystem");
    //         return ESP_FAIL;
    //     }
    //     else if (ret == ESP_ERR_NOT_FOUND) {
    //         ESP_LOGI(TAG, "Failed to find SPIFFS partition");
    //         return ESP_FAIL;
    //     }
    //     else {
    //         ESP_LOGI(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
    //         return ESP_FAIL;
    //     }   
    // }

    FileWrite(userName, SPIFFS_USER_NAME_PATH, SPIFFS_FILE_WRITE);
    FileWrite(wifi, SPIFFS_WIFI_PATH, SPIFFS_FILE_WRITE);
    FileWrite(password, SPIFFS_PASSWORD_PATH, SPIFFS_FILE_WRITE);

    esp_vfs_spiffs_unregister(NULL);
    ESP_LOGI(TAG, "[SetWiFiInformation] End");

    esp_restart();
    return ESP_OK;
}

/********************************************************************************
* GetWiFiInformation
* 
* Description : 전원 On 시 SPIFFS에 저장되어 있는 
*               Wifi 정보와 Password 정보를 가져옴
********************************************************************************/

esp_err_t GetWiFiInformation()
{
    ESP_LOGI(TAG, "[GetWiFiInformation] Start");
    esp_err_t ret = esp_vfs_spiffs_register(&spiffsConfig);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGI(TAG, "Failed to mount or format filesystem");
            return ESP_FAIL;
        }
        else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGI(TAG, "Failed to find SPIFFS partition");
            return ESP_FAIL;
        }
        else {
            ESP_LOGI(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
            return ESP_FAIL;
        }   
    }

    FILE* f;

    f = fopen(SPIFFS_WIFI_PATH, SPIFFS_FILE_READ);
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    
    fgets(wifiId, sizeof(wifiId), f);

    f = fopen(SPIFFS_PASSWORD_PATH, SPIFFS_FILE_READ);
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }

    fgets(wifiPassword, sizeof(wifiPassword), f);

    ESP_LOGI(TAG, "[GetWiFiInformation] SSID : %s, Password : %s", wifiId, wifiPassword);
    
    fclose(f);

    esp_vfs_spiffs_unregister(NULL);
    ESP_LOGI(TAG, "[GetWiFiInformation] End");

    return ESP_OK;
}