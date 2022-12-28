/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 a-pushkin on GitHub
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <pico/stdlib.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "picoprobe_config.h"
#include "led.h"



static TaskHandle_t task_led;

static bool         msc_connected;
static bool         dapv1_connected;
static bool         dapv2_connected;
static bool         target_found;
static unsigned     rtt_flash_cnt;
static uint64_t     uart_data_trigger;
static uint64_t     rtt_data_trigger;



void led_thread(void *ptr)
{
    for (;;) {
        if ( !target_found) {
            // -> 5Hz blinking
            gpio_put(PICOPROBE_LED, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(PICOPROBE_LED, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        else if (dapv1_connected) {
            // -> LED on, off for 100ms once per second
            gpio_put(PICOPROBE_LED, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(PICOPROBE_LED, 1);
            vTaskDelay(pdMS_TO_TICKS(900));
        }
        else if (dapv2_connected) {
            // -> LED on, off for 100ms twice per second
            gpio_put(PICOPROBE_LED, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(PICOPROBE_LED, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(PICOPROBE_LED, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(PICOPROBE_LED, 1);
            vTaskDelay(pdMS_TO_TICKS(700));
        }
        else if (msc_connected) {
            // -> LED on, off for 100ms thrice per second
            gpio_put(PICOPROBE_LED, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(PICOPROBE_LED, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(PICOPROBE_LED, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(PICOPROBE_LED, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(PICOPROBE_LED, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(PICOPROBE_LED, 1);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        else if (time_us_64() - uart_data_trigger < 5000000) {
            // -> slow flashing
            gpio_put(PICOPROBE_LED, 1);
            vTaskDelay(pdMS_TO_TICKS(300));
            gpio_put(PICOPROBE_LED, 0);
            vTaskDelay(pdMS_TO_TICKS(700));
        }
        else {
            // -> LED off, flashes once per second for 20ms if no RTT control block found
            // -> LED off, flashes twice per second for 20ms if RTT control block found
            // -> LED off, flashes thrice per second for 20ms if RTT data received
            unsigned flash_cnt = rtt_flash_cnt;
            if (time_us_64() - rtt_data_trigger < 5000000) {
                flash_cnt = 3;
            }
            for (unsigned u = 0;  u < flash_cnt;  ++u) {
                gpio_put(PICOPROBE_LED, 1);
                vTaskDelay(pdMS_TO_TICKS(20));
                gpio_put(PICOPROBE_LED, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            gpio_put(PICOPROBE_LED, 0);
            vTaskDelay(pdMS_TO_TICKS(1000 - rtt_flash_cnt * 220));
        }
    }
}   // led_thread



/**
 * Set state of LED.
 *
 * \attention
 *    This function is also called form interrupt!
 */
void led_state(led_state_t state)
{
    switch (state) {
        case LS_TARGET_FOUND:
            target_found  = true;
            rtt_flash_cnt = 1;
            break;

        case LS_NO_TARGET:
            target_found  = false;
            break;

        case LS_MSC_CONNECTED:
            msc_connected = true;
            rtt_flash_cnt = 0;
            break;

        case LS_MSC_DISCONNECTED:
            msc_connected = false;
            break;

        case LS_DAPV1_CONNECTED:
            dapv1_connected = true;
            rtt_flash_cnt = 0;
            break;

        case LS_DAPV1_DISCONNECTED:
            dapv1_connected = false;
            break;

        case LS_DAPV2_CONNECTED:
            dapv2_connected = true;
            rtt_flash_cnt = 0;
            break;

        case LS_DAPV2_DISCONNECTED:
            dapv2_connected = false;
            break;

        case LS_RTT_CB_FOUND:
            rtt_flash_cnt = 2;
            break;

        case LS_RTT_DATA:
            rtt_data_trigger = time_us_64();
            break;

        case LS_UART_DATA:
            uart_data_trigger = time_us_64();
            break;

        default:
            break;
    }
}   // led_state



/**
 * Initialize the LED task.
 */
void led_init(uint32_t task_prio)
{
    picoprobe_debug("led_init()\n");

    gpio_init(PICOPROBE_LED);
    gpio_set_dir(PICOPROBE_LED, GPIO_OUT);
    gpio_put(PICOPROBE_LED, 1);

    xTaskCreate(led_thread, "LED", configMINIMAL_STACK_SIZE, NULL, task_prio, &task_led);
}   // led_init
