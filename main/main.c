#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"
// #include "led_strip_encoder.h"
#include "led_matrix.h"

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define RMT_LED_STRIP_GPIO_NUM      19

#define LED_COLS                    4
#define LED_ROWS                    1
#define EXAMPLE_CHASE_SPEED_MS      100

static const char *TAG = "led_matrix_main";

// Condition for enable/disable crimson_azure_flow
bool is_crimson_azure_flow_enabled = true;

void crimson_azure_flow_task(void* pvParameters) {
    crimson_azure_flow(EXAMPLE_CHASE_SPEED_MS, &is_crimson_azure_flow_enabled);
    vTaskDelete(NULL);
}

void app_main(void)
{
    init_matrix(RMT_LED_STRIP_GPIO_NUM, LED_ROWS, LED_COLS);
    ESP_LOGI(TAG, "Before start leds");

    xTaskCreate(&crimson_azure_flow_task, "crimson_azure_flow_task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "Before disable leds");
    vTaskDelay(pdMS_TO_TICKS(5000));
    is_crimson_azure_flow_enabled = false;
    ESP_LOGI(TAG, "After disable leds");
}
