#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_log.h"

#include "esp_system.h"
#include "esp_netif.h"
#include "mqtt_client.h"
#include "esp_spiffs.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "../inc/common.h"
#include "../inc/mqtt.h"
#include "../inc/file.h"

static const char* TAG = "MQTT";

static esp_mqtt_client_handle_t mqttClient;

esp_mqtt_client_config_t mqttConfig = {
    .uri = MQTT_SERVER_ADDRESS,
    .port = 1883,
};

void MQTTInit()
{
    ESP_LOGI(TAG, "[MQTTInit] Start");
    nvs_flash_init();
    tcpip_adapter_init();
    esp_netif_init();
    esp_event_loop_create_default();

    MQTTStart();
    ESP_LOGI(TAG, "[MQTTInit] End");
}

static void MQTTEventHandler(void *args, esp_event_base_t base, int32_t eventId, void* eventData)
{
    ESP_LOGI(TAG, "[MQTTEventHandler] Start");
    esp_mqtt_event_handle_t event = eventData;
    //esp_mqtt_client_handle_t client = event->client;
    mqttClient = event->client;
    int messageId;

    switch((esp_mqtt_event_id_t)eventId) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "[MQTTEventHandler] MQTT_EVENT_CONNECTED");
            PublishUserName();
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "[MQTTEventHandler] MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            break;
        case MQTT_EVENT_PUBLISHED:
            break;
        case MQTT_EVENT_DATA:
        break;
        case MQTT_EVENT_ERROR:
            if(event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                ESP_LOGI(TAG, "[MQTTEventHandler] MQTT_EVENT_ERROR");
            }
        default:
            break;
    }
    ESP_LOGI(TAG, "[MQTTEventHandler] End");
}

static void MQTTStart(void)
{
    ESP_LOGI(TAG, "[MQTTStart] Start");

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqttConfig);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, MQTTEventHandler, NULL);
    esp_mqtt_client_start(client);

    ESP_LOGI(TAG, "[MQTTStart] End");
}

void MQTTPublish(char *topic, char *payload)
{
    ESP_LOGI(TAG, "[MQTTPublish] Start");
    esp_mqtt_client_publish(mqttClient, topic, payload, 0, 1, 0);

    ESP_LOGI(TAG, "[MQTTPublish] End");
}

esp_err_t PublishUserName()
{
    ESP_LOGI(TAG, "[PublishUserName] Start");
    
    char userName[32];
    char sendName[32];

    static esp_vfs_spiffs_conf_t spiffsConfig = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

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

    f = fopen(SPIFFS_USER_NAME_PATH, SPIFFS_FILE_READ);
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    
    fgets(userName, sizeof(userName), f);
    snprintf(sendName, "{\"name\":%s}", userName);

    ESP_LOGI(TAG, "[PublishUserName] Send UserName : %s", sendName);
    
    MQTTPublish(MQTT_TOPIC_LIST_NAME, sendName);

    fclose(f);

    esp_vfs_spiffs_unregister(NULL);
    ESP_LOGI(TAG, "[PublishUserName] End");

    return ESP_OK;
}