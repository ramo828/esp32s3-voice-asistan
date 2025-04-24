// components/buzzer/include/buzzer.h
#pragma once

#include <stdint.h>

// Bu bileşende kullandığımız API’lar C kodu, ESP_ERR_CHECK makrolarını da
#include "esp_err.h"

// GPIO ve LEDC tipleri için prototipleri burada kullanıyoruz,  
// ama gerçek başlıklar buzzer.c içinde include edilecek.
#define BUZZER_PIN       GPIO_NUM_9

#define LEDC_MODE        LEDC_LOW_SPEED_MODE
#define LEDC_TIMER       LEDC_TIMER_0
#define LEDC_CHANNEL     LEDC_CHANNEL_0
#define LEDC_DUTY_RES    LEDC_TIMER_10_BIT
#define LEDC_DUTY        (500)

void buzzer_init(void);
void buzzer_beep(uint32_t freq_hz, uint32_t duration_ms);
