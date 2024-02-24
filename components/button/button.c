#include <stdio.h>
#include "button.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "button_component";

void button_setup(gpio_num_t gpio_num) {
    gpio_config_t gpio_conf = {
        .pin_bit_mask = (1ULL << gpio_num),
        .mode = GPIO_MODE_INPUT, // 1
        .pull_up_en = GPIO_PULLUP_ENABLE, // 1
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // 0
        .intr_type = GPIO_INTR_POSEDGE,
    };
    gpio_config(&gpio_conf);
}

int last_state = 1;

void on_button_press(on_press_button_t cb, gpio_num_t gpio_num) {
    while (1) {
        int current_state = gpio_get_level(gpio_num);
        
        if (current_state == 0 && current_state != last_state) {
            cb();
        }
        last_state = current_state;
         // Обновление значения предыдущего состояния
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


