#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "./header/boom_barrier.h"
#include "./header/http_server.h"
#include "./header/http_server.h"
#include "./header/ethernet_connect.h"
#include "./header/spiffs_create.h"


void app_main(void){
    gpio_set_direction(GPIO_NUM_21, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_19, GPIO_MODE_OUTPUT);
    spiffs_start();

    up_function(read_time());


    // wifi_init_sta();
    ethernet_task_start();

    ESP_LOGW(TAG, "The program reads the barrier time as: %d", read_time());

    TaskHandle_t xHandle = NULL;
    static uint8_t ucParameterToPass;
    // http_server_start();
}
