#include "esp_log.h"
#include "driver/gpio.h"
#include "../header/boom_barrier.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "header/spiffs_create.h"

// STOP PIN in this function is defined in the boom_barrier.h function
// This function is to stop any further action on the barrier
void stop_function() {
    ESP_LOGW("Boom Barrier", "Stop function called");
    gpio_set_level(STOP_PIN, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(STOP_PIN, 1);
}


// UP PIN in this function is defined in the boom_barrier.h function
// This function is to trigger the function to move up the barrier
void up_function(int value){
    ESP_LOGW("Boom Barrier", "Up function Called");
    gpio_set_level(UP_PIN, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(UP_PIN, 1);
    vTaskDelay((value-100) / portTICK_PERIOD_MS);
    stop_function();
}


// DOWN PIN in this function is defined in the boom_barrier.h function
// This function is to trigger the function to move down the barrier
void down_function(int value) {
    ESP_LOGW("Boom Barrier", "Down function called");
    gpio_set_level(DOWN_PIN, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(DOWN_PIN, 1);
    vTaskDelay((value-100) / portTICK_PERIOD_MS);
    stop_function();
}

