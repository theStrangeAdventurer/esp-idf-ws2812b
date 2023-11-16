#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "led_matrix.h"

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define RMT_LED_STRIP_GPIO_NUM      19

#define LED_COLS                    16
#define LED_ROWS                    16
#define EXAMPLE_CHASE_SPEED_MS      100

// Condition for enable/disable crimson_azure_flow
bool is_crimson_azure_flow_enabled = false;
bool is_torch_enabled = true;
int torch_level = 0; // Off
int mode = 1; // 0 - default,  1 - warm

void crimson_azure_flow_task(void* pvParameters) {
    crimson_azure_flow(EXAMPLE_CHASE_SPEED_MS, &is_crimson_azure_flow_enabled);
    vTaskDelete(NULL);
}

void torch_task(void* pvParameters) {
    torch(&is_torch_enabled, &torch_level, &mode);
    vTaskDelete(NULL);
}

void torch_change_level_task(void* pvParameters) {
    while (1)
    {
       vTaskDelay(pdMS_TO_TICKS(5000));
       torch_level = (torch_level + 1) % 4;
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    init_matrix(RMT_LED_STRIP_GPIO_NUM, LED_ROWS, LED_COLS);

    xTaskCreate(&crimson_azure_flow_task, "crimson_azure_flow_task", 4096, NULL, 5, NULL);
    xTaskCreate(&torch_change_level_task, "torch_change_level_task", 4096, NULL, 5, NULL);
    xTaskCreate(&torch_task, "torch_task", 4096, NULL, 5, NULL);
}
