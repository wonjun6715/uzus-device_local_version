#include <stdio.h>
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../inc/gpio.h"
#include "../inc/common.h"
#include "../inc/mqtt.h"

static const char* TAG = "GPIO";

/********************************************************************************
* GPIOInit
* 
* Description : GPIO Pin I/O 정의
********************************************************************************/

void GPIOInit()
{
    ESP_LOGI(TAG, "[GPIOInit] Start");

    gpio_set_direction(LED_RELAY, OUTPUT);
    
    ESP_LOGI(TAG, "[GPIOInit] End");
}
