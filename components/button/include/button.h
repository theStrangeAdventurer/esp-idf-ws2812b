#pragma once
#include "driver/gpio.h"

void button_setup(int gpio_num);

typedef void (*on_press_button_t)(void);

void on_button_press(on_press_button_t cb, gpio_num_t gpio_num);
