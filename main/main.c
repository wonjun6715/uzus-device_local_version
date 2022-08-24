#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"

#include "esp_log.h"

#include "./inc/file.h"
#include "./inc/gpio.h"
#include "./inc/wifi.h"
#include "./inc/mqtt.h"
#include "./inc/adc.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

static const char* TAG = "MAIN";



void app_main(void)
{   
    int actionStatus = 1;
    
    ESP_LOGI(TAG, "[app_main] Start");
    GPIOInit();
    WiFiInit();
    MQTTInit();
    ADCInit();

    while(1) {
        if(GetADCValue(ADC1_CHANNEL_6) <= 20) {
            gpio_set_level(LED_RELAY, 1);
            if(actionStatus == 1) {
                MQTTPublish(MQTT_TOPIC_LIST_ACTION, MQTT_PUBLISH_ACTION);
                actionStatus = 0;
            }
        }
        else {
            actionStatus = 1;
            gpio_set_level(LED_RELAY, 0);
        }
    }
    ESP_LOGI(TAG, "[app_main] End");
}