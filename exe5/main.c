/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>


#include <stdio.h>
#include <string.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

const int BTN_R = 0;
const int BTN_Y = 1;


QueueHandle_t xQueueButId;
SemaphoreHandle_t xSemaphore_r;
SemaphoreHandle_t xSemaphore_y;

void btn_callback(uint gpio, uint32_t events) {
    if (events == GPIO_IRQ_EDGE_FALL) {
        int btn_id;
        if (gpio == BTN_PIN_R) {
            btn_id = BTN_R;
            xQueueSendFromISR(xQueueButId, &btn_id, 0);
        } else if (gpio == BTN_PIN_Y) {
            btn_id = BTN_Y;
            xQueueSendFromISR(xQueueButId, &btn_id, 0);
        }
    }
}

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    bool pisca =false;

    while (true) {
        if (!pisca){
            if (xSemaphoreTake(xSemaphore_r, portMAX_DELAY) == pdTRUE) {
                pisca = true;
        }
    } else {
        if (xSemaphoreTake(xSemaphore_r, 0) == pdTRUE) {
                pisca = false;
                gpio_put(LED_PIN_R, 0);
            } else {
                gpio_put(LED_PIN_R, 1);
                vTaskDelay(pdMS_TO_TICKS(100));
                gpio_put(LED_PIN_R, 0);
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }   
    }
}


void led_2_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    bool pisca =false;

    while (true) {
        if (!pisca){
            if (xSemaphoreTake(xSemaphore_y, portMAX_DELAY) == pdTRUE) {
                pisca = true;
        }
    } else {
        if (xSemaphoreTake(xSemaphore_y, 0) == pdTRUE) {
                pisca = false;
                gpio_put(LED_PIN_Y, 0);
            } else {
                gpio_put(LED_PIN_Y, 1);
                vTaskDelay(pdMS_TO_TICKS(100));
                gpio_put(LED_PIN_Y, 0);
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }   
    }
}

void btn_task(void *p) {
    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true,
                                       &btn_callback);
    
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

    int btn_id;

    while (true) {
        if (xQueueReceive(xQueueButId, &btn_id, portMAX_DELAY) == pdTRUE) {
            if (btn_id == BTN_R) {
                xSemaphoreGive(xSemaphore_r);
            } else if (btn_id == BTN_Y) {
                xSemaphoreGive(xSemaphore_y);
            }
        }
    }
}

int main() {
    stdio_init_all();
    printf("Start RTOS \n");

    xQueueButId = xQueueCreate(32, sizeof(int));
    xSemaphore_r = xSemaphoreCreateBinary();
    xSemaphore_y = xSemaphoreCreateBinary();

    xTaskCreate(btn_task, "BTN_Task", 256, NULL, 1, NULL);
    xTaskCreate(led_1_task, "LED_Task_R", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED_Task_Y", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
