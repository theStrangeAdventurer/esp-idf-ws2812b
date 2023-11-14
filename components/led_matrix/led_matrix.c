#include "led_matrix.h"
#include <stdint.h>
#include <string.h>
#include "led_strip_encoder.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define BIT_PER_ONE_ADDRESS_LED 24

static const char *TAG = "led_matrix_component";

// FIXME: Дописать функцию для цветового сдвига по HUE кругу
// // Проход по всем рядам и плавное изменение цвета от рядя к ряду (Зацикленное)
// while (1) {
//     for (int i = 0; i < LED_PER_COL; i++) {
//         hue = i * 360 / EXAMPLE_LED_NUMBERS + start_rgb;
//         led_strip_hsv2rgb(hue, 100, BRIGHTNESS, &red, &green, &blue);
//         for (int j = 0; j < (LED_PER_COL * PIXELS_PER_CELL); j+= PIXELS_PER_CELL) {
//             int pixel = (i * LED_PER_COL * PIXELS_PER_CELL) + j;
//             // printf("Led: %d\n", cell);
//             printf("Color: r=%d, g=%d, b=%d\n", (int)red, (int)green, (int)blue);
//             led_strip_pixels[pixel] = red;
//             led_strip_pixels[pixel + 1] = green;
//             led_strip_pixels[pixel + 2] = blue;
//         }
//         // Flush RGB values to LEDs
//         // Каждое число 4 байта
//         ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, EXAMPLE_LED_NUMBERS * 4, &tx_config));
        // ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
//         vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
        // memset(led_strip_pixels, 0, sizeof(led_strip_pixels));
//         ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, EXAMPLE_LED_NUMBERS, &tx_config));
//         ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
//         vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
//     }
//     start_rgb += 12;
// }

// FIXME: Дописать функцию моргания светодиодов 0 -> 255 -> 0
void increase_brightness(uint8_t *pval, const uint8_t increase, uint8_t *direction) {
    // set_pixel_color(pPixels, 0, 0, brightness, 0);
    if ((*pval + increase) >= 50) {
        *direction = -1;
    }
    
    if ((*pval - increase) <= 0) {
        *direction = 1;
    }

    if (*direction == 1) {
        *pval += increase;
    } else {
        *pval -= increase;  
    }
    return;
}

void set_pixel_color(uint8_t * pPixels,  int offset, int r, int g, int b)
{
    int _offset = offset * 3; // Три пикселя в каждом светодиоде
    // g, r, b - Реальная последовательность, а не rgb
    pPixels[_offset] = g;
    pPixels[_offset + 1] = r;
    pPixels[_offset + 2] = b;
}

void led_strip_hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r, uint32_t *g, uint32_t *b)
{
    h %= 360; // h -> [0,360]
    uint32_t rgb_max = v * 2.55f;
    uint32_t rgb_min = rgb_max * (100 - s) / 100.0f;

    uint32_t i = h / 60;
    uint32_t diff = h % 60;

    // RGB adjustment amount by hue
    uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

    switch (i) {
    case 0:
        *r = rgb_max;
        *g = rgb_min + rgb_adj;
        *b = rgb_min;
        break;
    case 1:
        *r = rgb_max - rgb_adj;
        *g = rgb_max;
        *b = rgb_min;
        break;
    case 2:
        *r = rgb_min;
        *g = rgb_max;
        *b = rgb_min + rgb_adj;
        break;
    case 3:
        *r = rgb_min;
        *g = rgb_max - rgb_adj;
        *b = rgb_max;
        break;
    case 4:
        *r = rgb_min + rgb_adj;
        *g = rgb_min;
        *b = rgb_max;
        break;
    default:
        *r = rgb_max;
        *g = rgb_min;
        *b = rgb_max - rgb_adj;
        break;
    }
}

/**
 * Global variables
*/
rmt_channel_handle_t led_chan = NULL;
uint8_t * pled_strip_pixels = NULL;
rmt_encoder_handle_t led_encoder = NULL;
rmt_transmit_config_t tx_config = {
    .loop_count = 0, // no transfer loop
};

int led_numbers = 0;

/**
 * Init function
*/
void init_matrix(int gpio_num, int led_per_col, int led_per_row)
{
    led_numbers = led_per_col * led_per_row;
    pled_strip_pixels = (uint8_t*)malloc(led_numbers * sizeof(uint8_t));

    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = 4, // select source clock ?? I don't know what mean that number
        .gpio_num = gpio_num,
        .mem_block_symbols = 64, // increase the block size can make the LED less flickering
        .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
        .trans_queue_depth = 4, // set the number of transactions that can be pending in the background
    };

    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

    ESP_LOGI(TAG, "Install led strip encoder");

    led_strip_encoder_config_t encoder_config = {
        .resolution = RMT_LED_STRIP_RESOLUTION_HZ,
    };

    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

    ESP_LOGI(TAG, "Enable RMT TX channel");
    ESP_ERROR_CHECK(rmt_enable(led_chan));

    ESP_LOGI(TAG, "Start LED rainbow chase");
    
    memset(pled_strip_pixels, 0, led_numbers * sizeof(uint8_t)); // FIXME: добавить метод destroy с освобождением этой памяти
}

/**
 * Effects
*/
void crimson_azure_flow(int chase_speed)
{
    uint8_t brightness = 0;
    uint8_t brightness_dir = 1;
    uint8_t brightness_blue = 0;
    uint8_t brightness_blue_dir = 1;
    uint8_t increase = 2;

    while (1) {
        for (int i = 0; i < led_numbers; i++) {
            set_pixel_color(pled_strip_pixels, i, brightness, 0, brightness_blue);
            ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, pled_strip_pixels, led_numbers * BIT_PER_ONE_ADDRESS_LED, &tx_config));
            ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
            vTaskDelay(pdMS_TO_TICKS(chase_speed));
        }
        increase_brightness(&brightness, increase, &brightness_dir);
        increase_brightness(&brightness_blue, increase * 2, &brightness_blue_dir);
        vTaskDelay(pdMS_TO_TICKS(chase_speed));
    }
}
