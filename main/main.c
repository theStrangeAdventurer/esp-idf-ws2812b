#include <string.h>
#include <stdbool.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/rmt_tx.h"
#include "led_matrix.h"
#include "button.h"

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define RMT_LED_STRIP_GPIO_NUM      19
#define BUTTON_GPIO_NUM             18

#define LED_COLS                    4
#define LED_ROWS                    1
#define EXAMPLE_CHASE_SPEED_MS      100

bool is_torch_enabled = true;
int torch_level = 0; // Off
int mode = 0; // 0 - default,  1 - warm

static const char *TAG = "led_matrix_main";

void crimson_azure_flow_task(void* pvParameters) {
    crimson_azure_flow(EXAMPLE_CHASE_SPEED_MS);
}

void torch_task(void* pvParameters) {
    torch(1, 1);
}

TaskHandle_t xAzureHandle = NULL;
TaskHandle_t xTorchHandle = NULL;

void button_press_cb() {
    mode = (mode + 1) % 3;
    ESP_LOGI(TAG, "Button pressed >> mode = %d", mode);

    if (mode == 1) {
        ESP_LOGI(TAG, "mode: azure");
        xTaskCreate(
            &crimson_azure_flow_task,
            "crimson_azure_flow_task",
            /*  Размер стека, выделяемого задаче, не в байтах,а в словах (хз что за слова) */ 4096,
            /* void* pvParameters которые приходят в обработчик crimson_azure_flow_task */ NULL,
            /* Приоритет */ 5,
            /* Дескриптор создаваемой задачи, может быть в последствии использован в качестве ссылки на задачу при вызове других функций API RTOS */&xAzureHandle
        );
    } else {
        if( xAzureHandle != NULL ) {
            ESP_LOGI(TAG, "mode: not azure >> delete task");
            vTaskDelete( xAzureHandle );
            xAzureHandle = NULL;
            reset_matrix();
        }
    }

    if (mode == 2) {
        ESP_LOGI(TAG, "mode: torch");
        xTaskCreate(
            &torch_task,
            "torch_task",
            /*  Размер стека, выделяемого задаче, не в байтах,а в словах (хз что за слова) */ 4096,
            /* void* pvParameters которые приходят в обработчик crimson_azure_flow_task */ NULL,
            /* Приоритет */ 5,
            /* Дескриптор создаваемой задачи, может быть в последствии использован в качестве ссылки на задачу при вызове других функций API RTOS */&xTorchHandle
        );
    } else {
        if( xTorchHandle != NULL ) {
            ESP_LOGI(TAG, "mode: not torch >> delete task");
            vTaskDelete( xTorchHandle );
            xTorchHandle = NULL;
            reset_matrix();
        }
    }
    
}

void button_press_task(void* pvParameters) {
    on_button_press(button_press_cb, BUTTON_GPIO_NUM);
}

void app_main(void)
{
    init_matrix(RMT_LED_STRIP_GPIO_NUM, LED_ROWS, LED_COLS);
    button_setup(BUTTON_GPIO_NUM);
    xTaskCreate(&button_press_task, "button_press_task", 4096, NULL, 5, NULL);
}
