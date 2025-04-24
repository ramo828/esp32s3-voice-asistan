#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_wn_iface.h"
#include "esp_wn_models.h"
#include "esp_afe_sr_iface.h"
#include "esp_afe_sr_models.h"
#include "esp_mn_iface.h"
#include "esp_mn_models.h"
#include "esp_board_init.h"
#include "speech_commands_action.h"
#include "model_path.h"
#include "esp_process_sdkconfig.h"
#include "driver/ledc.h"
#include <led_strip.h>
#include <driver/rmt_types_legacy.h>
#include "driver/rmt.h"
#include "driver/rmt_tx.h"
#include "buzzer.h"

// --------------------
// ADRESLENEBİLİR LED TANIMLARI
// --------------------
#define RGB_STRIP_GPIO        GPIO_NUM_48
#define RGB_STRIP_RMT_CHAN    RMT_CHANNEL_0
#define RGB_STRIP_HZ          10000000  // 10 MHz
// Global ya da static değişkenler
static bool red_status   = false;
static bool green_status = false;
static bool blue_status  = false;

static led_strip_t *strip = NULL;

static esp_afe_sr_iface_t *afe_handle = NULL;
static volatile int task_flag = 0;
static int play_voice = -2;
int detect_flag = 0;
srmodel_list_t *models = NULL;


// 💡 Eğer ses çalma kullanılmayacaksa dummy fonksiyon:
void bsp_audio_play(const char *filename, int blocking) {
    printf("Dummy audio play: %s\n", filename);
}

void play_music(void *arg) {
    while (task_flag) {
        switch (play_voice) {
            case -2:
                vTaskDelay(10);
                break;
            case -1:
                wake_up_action();
                play_voice = -2;
                break;
            default:
                speech_commands_action(play_voice);
                play_voice = -2;
                break;
        }
    }
    vTaskDelete(NULL);
}

void feed_Task(void *arg) {
    esp_afe_sr_data_t *afe_data = arg;
    int audio_chunksize = afe_handle->get_feed_chunksize(afe_data);
    int feed_channel = esp_get_feed_channel();
    int16_t *i2s_buff = malloc(audio_chunksize * sizeof(int16_t) * feed_channel);
    assert(i2s_buff);

    while (task_flag) {
        esp_get_feed_data(false, i2s_buff, audio_chunksize * sizeof(int16_t) * feed_channel);
        afe_handle->feed(afe_data, i2s_buff);
    }

    free(i2s_buff);
    vTaskDelete(NULL);
}

void detect_Task(void *arg) {
    esp_afe_sr_data_t *afe_data = arg;
    int afe_chunksize = afe_handle->get_fetch_chunksize(afe_data);
    char *mn_name = esp_srmodel_filter(models, ESP_MN_PREFIX, ESP_MN_ENGLISH);
    esp_mn_iface_t *multinet = esp_mn_handle_from_name(mn_name);
    model_iface_data_t *model_data = multinet->create(mn_name, 6000);

    multinet->print_active_speech_commands(model_data);
    esp_mn_commands_update_from_sdkconfig(multinet, model_data);

    printf("Speech detection started...\n");

    while (task_flag) {
        afe_fetch_result_t *res = afe_handle->fetch(afe_data);
        if (!res || res->ret_value == ESP_FAIL) {
            printf("AFE fetch error\n");
            break;
        }

        if (res->wakeup_state == WAKENET_DETECTED) {
            printf("Wake word detected!\n");
            buzzer_beep(262, 100);   // 500 Hz, 100 ms bip
            multinet->clean(model_data);
        } else if (res->wakeup_state == WAKENET_CHANNEL_VERIFIED) {
            play_voice = -1;
            detect_flag = 1;
            printf("Wake channel verified, ID: %d\n", res->trigger_channel_id);
        }

        if (detect_flag >= 1) {
            esp_mn_state_t mn_state = multinet->detect(model_data, res->data);

            if (mn_state == ESP_MN_STATE_DETECTING) continue;

            if (mn_state == ESP_MN_STATE_DETECTED) {
                esp_mn_results_t *mn_result = multinet->get_results(model_data);
                int cmd_id = mn_result->command_id[0];
                detect_flag = 2;
                buzzer_beep(500, 100);   // 500 Hz, 100 ms bip
                printf("Detected Command: %s (ID: %d)\n", mn_result->string, cmd_id);


                switch (cmd_id) {
                    case 0:
                        printf("Turn on red light\n"); 
                        red_status = true;              
                        break;
                    case 1:
                        printf("Turn on green light\n"); 
                        green_status = true;                   
                        break;
                    case 2:
                        printf("Turn on blue light\n"); 
                        blue_status = true;   
                        break;
                    case 3:
                        printf("Turn off red light\n"); 
                        red_status = false;   
                        break;
                    case 4:
                        printf("Turn off green light\n"); 
                        green_status = false;   
                        break;
                    case 5:
                        printf("Turn off blue light\n"); 
                        blue_status = false;   
                        break;
                    case 6: 
                        printf("Turn on the light\n"); 
                        red_status = true;   
                        green_status = true;
                        blue_status = true;
                        break;
                    case 7: 
                        printf("Turn off the light\n"); 
                        red_status = false;
                        green_status = false;
                        blue_status = false;
                        break;
                   
                    case 32:
                        break;
                    default:
                        printf("Unknown Command ID: %d\n", cmd_id);
                        buzzer_beep(349, 100);   // 500 Hz, 100 ms bip
                        break;
                }
                 strip->set_pixel(strip, 0, red_status ? 255 : 0, green_status ? 255 : 0,   blue_status ? 255 : 0);
                 strip->refresh(strip, 100);   
            }

            if (mn_state == ESP_MN_STATE_TIMEOUT) {
                esp_mn_results_t *mn_result = multinet->get_results(model_data);
                printf("Timeout. Last heard: %s\n", mn_result->string);
                afe_handle->enable_wakenet(afe_data);
                detect_flag = 0;
                printf("Awaiting wake word...\n");
            }
        }
    }

    multinet->destroy(model_data);
    vTaskDelete(NULL);
}

void app_main() {
  {
        // Önce LEDC’yi başlat
        buzzer_init();
        // 2.a) RMT TX kanalını yapılandır
        rmt_config_t cfg = RMT_DEFAULT_CONFIG_TX(RGB_STRIP_GPIO, RGB_STRIP_RMT_CHAN);
        cfg.clk_div = 8; // 80MHz / 8 = 10MHz WS2812 için
        ESP_ERROR_CHECK(rmt_config(&cfg));
        ESP_ERROR_CHECK(rmt_driver_install(cfg.channel, 0, 0));

        // 2.b) LED şeridini oluştur
        led_strip_config_t s_cfg = LED_STRIP_DEFAULT_CONFIG(1, (led_strip_dev_t)cfg.channel);
        strip = led_strip_new_rmt_ws2812(&s_cfg);
        assert(strip && "LED strip init failed");
        // 2.c) Başlangıçta tüm LED’leri kapat
        ESP_ERROR_CHECK(strip->clear(strip, 100));
    }

    models = esp_srmodel_init("model");
    ESP_ERROR_CHECK(esp_board_init(AUDIO_HAL_16K_SAMPLES, 1, 16));

#if CONFIG_IDF_TARGET_ESP32
    printf("This demo only supports ESP32S3\n");
    return;
#else
    afe_handle = (esp_afe_sr_iface_t *)&ESP_AFE_SR_HANDLE;
#endif

    afe_config_t afe_config = AFE_CONFIG_DEFAULT();
    afe_config.wakenet_model_name = esp_srmodel_filter(models, ESP_WN_PREFIX, NULL);

#if defined(CONFIG_ESP32_S3_BOX_BOARD) || defined(CONFIG_ESP32_S3_EYE_BOARD) || defined(CONFIG_ESP32_S3_DEVKIT_C)
    afe_config.aec_init = false;
    #if defined(CONFIG_ESP32_S3_EYE_BOARD) || defined(CONFIG_ESP32_S3_DEVKIT_C)
        afe_config.pcm_config.total_ch_num = 2;
        afe_config.pcm_config.mic_num = 1;
        afe_config.pcm_config.ref_num = 1;
    #endif
#endif

    esp_afe_sr_data_t *afe_data = afe_handle->create_from_config(&afe_config);

    task_flag = 1;

    xTaskCreatePinnedToCore(detect_Task, "detect", 8 * 1024, afe_data, 5, NULL, 1);
    xTaskCreatePinnedToCore(feed_Task, "feed", 8 * 1024, afe_data, 5, NULL, 0);
    xTaskCreatePinnedToCore(play_music, "play", 4 * 1024, NULL, 5, NULL, 1);
}
