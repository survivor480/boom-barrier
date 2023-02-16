#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"




#define STOP_PIN GPIO_NUM_2
#define UP_PIN GPIO_NUM_18
#define DOWN_PIN GPIO_NUM_19



void stop_function() {
    ESP_LOGD("Boom Barrier", "Stop function called");
    gpio_set_direction(STOP_PIN, 1);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    gpio_set_direction(STOP_PIN, 0);
}



void up_function(){
    ESP_LOGD("Boom Barrier", "Up function Called");
    gpio_set_direction(UP_PIN, 1);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    gpio_set_direction(UP_PIN, 0);
    stop_function();
}


void down_function() {
    ESP_LOGD("Boom Barrier", "Down function called");
    gpio_set_direction(DOWN_PIN, 1);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    gpio_set_direction(DOWN_PIN, 0);
    stop_function();
}


void boom_barrier_function(){
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    up_function();
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    down_function();
}




void app_main(void)
{
    TaskHandle_t xHandle = NULL;
    xTaskCreatePinnedToCore(boom_barrier_function, "Boom Barrier", 500, NULL, 0, &xHandle, 0);
}
