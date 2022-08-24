#include <stdio.h>
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/adc.h"
#include "esp_adc_cal.h"

static const char* TAG = "ADC";

static esp_adc_cal_characteristics_t adc1_chars;

void ADCInit()
{
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 0, &adc1_chars);
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11));
}

uint32_t GetADCValue(int channel)
{
    ESP_LOGI(TAG, "[GetADCValue] Start");

    uint32_t voltage;
    uint32_t distance;

    voltage = esp_adc_cal_raw_to_voltage(adc1_get_raw(channel), &adc1_chars);
    distance = (27.61 / (voltage - 0.1696)) * 1000;

    ESP_LOGI(TAG, "[GetADCValue] Check Distance : %d", distance);

    vTaskDelay(pdMS_TO_TICKS(500)); 

    ESP_LOGI(TAG, "[GetADCValue] End");

    return distance;
}
