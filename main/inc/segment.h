#ifndef __SEGMENT_H__
#define __SEGMENT_H__

#include "driver/gpio.h"

#define SEGMENT_PIN_1          GPIO_NUM_27
#define SEGMENT_PIN_2          GPIO_NUM_32
#define SEGMENT_PIN_3          GPIO_NUM_5
#define SEGMENT_PIN_4          GPIO_NUM_16
#define SEGMENT_PIN_5          GPIO_NUM_4
#define SEGMENT_PIN_6          GPIO_NUM_26
#define SEGMENT_PIN_7          GPIO_NUM_18
#define SEGMENT_PIN_8          GPIO_NUM_17

#define SEGMENT_COM_PIN_1      GPIO_NUM_14
#define SEGMENT_COM_PIN_2      GPIO_NUM_25
#define SEGMENT_COM_PIN_3      GPIO_NUM_33
#define SEGMENT_COM_PIN_4      GPIO_NUM_19

void SegmentInit();
void SegmentShow(int number, int digit);

#endif