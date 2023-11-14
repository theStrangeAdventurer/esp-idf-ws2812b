#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "led_matrix.h"

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define RMT_LED_STRIP_GPIO_NUM      19

#define LED_PER_COL                 1
#define LED_PER_ROW                 4
#define EXAMPLE_CHASE_SPEED_MS      100

static const char *TAG = "led_matrix";

void app_main(void)
{
    init_matrix(RMT_LED_STRIP_GPIO_NUM, LED_PER_COL, LED_PER_ROW);
    crimson_azure_flow(EXAMPLE_CHASE_SPEED_MS);
}
