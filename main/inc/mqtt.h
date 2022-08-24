#ifndef __MQTT_H__
#define __MQTT_H__

#include "esp_event.h"

#define MQTT_SERVER_ADDRESS         "mqtt://192.168.35.33"

#define MQTT_TOPIC_LIST_NAME        "mqtt/name"
#define MQTT_TOPIC_LIST_ACTION      "mqtt/action"
#define MQTT_PUBLISH_ACTION         "action"

void MQTTInit();
static void MQTTEventHandler(void *args, esp_event_base_t base, int32_t eventId, void* eventData);
static void MQTTStart(void);
void MQTTPublish();
esp_err_t PublishUserName();

#endif