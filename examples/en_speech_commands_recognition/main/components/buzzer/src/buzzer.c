#include "buzzer.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"
#include "esp_log.h"

#define TAG "BUZZER"

static esp_timer_handle_t buzzer_timer;

// Buzzer kapanma fonksiyonu
static void buzzer_stop_callback(void* arg)
{
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    ESP_LOGI(TAG, "Buzzer kapatıldı");
}

// PWM yapılandırması
void buzzer_init(void)
{
    ledc_timer_config_t timer_cfg = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = 2000,
        .clk_cfg          = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_cfg));

    ledc_channel_config_t ch_cfg = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = BUZZER_PIN,
        .duty           = 0,
        .hpoint         = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch_cfg));

    // Timer sadece bir kez çalışacak şekilde tanımlanıyor
    const esp_timer_create_args_t timer_args = {
        .callback = &buzzer_stop_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "buzzer_timer"
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &buzzer_timer));
}

// Non-blocking beep
void buzzer_beep(uint32_t freq_hz, uint32_t duration_ms)
{
    ESP_ERROR_CHECK(ledc_set_freq(LEDC_MODE, LEDC_TIMER, freq_hz));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
    ESP_LOGI(TAG, "Buzzer aktif (%.1f kHz, %lu ms)", freq_hz / 1000.0, duration_ms);

    // Var olan timer çalışıyorsa durdur
    if (esp_timer_is_active(buzzer_timer)) {
        esp_timer_stop(buzzer_timer);
    }

    // Yeni sürede başlat
    ESP_ERROR_CHECK(esp_timer_start_once(buzzer_timer, duration_ms * 1000));
}
