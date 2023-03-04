#include "../header/spiffs_create.h"

void spiffs_start(void) {
    ESP_LOGI(FILE_SYSTEM_TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_vfs_spiffs_register(&conf);

    struct stat st;
    FILE* f;

    int barrier_close_time;

    if(stat("/spiffs/hello.txt", &st) != 0){
        ESP_LOGI(FILE_SYSTEM_TAG, "File does not exist");
        ESP_LOGI(FILE_SYSTEM_TAG, "Opening file");
        f = fopen("/spiffs/hello.txt", "w");
        fprintf(f, "Barrier time: 6000");
        fclose(f);
        ESP_LOGI(FILE_SYSTEM_TAG, "file written");
        barrier_close_time = 6000;
    } else {
        ESP_LOGW(FILE_SYSTEM_TAG, "File exists");
        f = fopen("/spiffs/hello.txt", "r");
        char line[60];
        char *barrier_time;
        fgets(line, sizeof(line), f);
        ESP_LOGI(FILE_SYSTEM_TAG, "File contents are: %s", line);
        fclose(f);

        barrier_time = strtok(line, ":");
        barrier_time = strtok(NULL, ":");
        barrier_close_time = atoi(barrier_time);
        ESP_LOGE(FILE_SYSTEM_TAG, "Barrier time is: %d",barrier_close_time);
    }
}


void barrier_time_change(int value){
    ESP_LOGI(FILE_SYSTEM_TAG, "Time Change Called");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_vfs_spiffs_register(&conf);
    
    FILE* f;

    char line[100];

    struct stat st;
    if (stat("/spiffs/hello.txt", &st) == 0) {
        // Delete it if it exists
        // ESP_LOGI(FILE_SYSTEM_TAG, "File exists. Removing file now");
        unlink("/spiffs/hello.txt");
        // if(stat("/spiffs/hello.txt", &st) != 0){
            // ESP_LOGI(FILE_SYSTEM_TAG, )
        // }
    }

    ESP_LOGI(FILE_SYSTEM_TAG, "The value to be changed to is: %d", value);

    f = fopen("/spiffs/hello.txt", "w+");
    fprintf(f, "Barrier Time: %d", value);

    vTaskDelay(10/portTICK_PERIOD_MS);

    fgets(line, sizeof(line), f);
    ESP_LOGI(FILE_SYSTEM_TAG, "File contents are: %s", line);
    fclose(f);

    esp_vfs_spiffs_unregister(conf.partition_label);
    ESP_LOGI(FILE_SYSTEM_TAG, "SPIFFS unmounted");
}

int read_time(){
    ESP_LOGI(FILE_SYSTEM_TAG, "Read Time Called");

    esp_vfs_spiffs_conf_t conf = {
        .base_path  = "/spiffs",
        .partition_label = NULL,
        .max_files  = 5,
        .format_if_mount_failed = true
    };

    esp_vfs_spiffs_register(&conf);

    FILE *f;

    f = fopen("/spiffs/hello.txt", "r");

    char line[60];
    char *barrier_time;
    fgets(line, sizeof(line), f);

    barrier_time = strtok(line, ":");
    barrier_time = strtok(NULL, ":");
    esp_vfs_spiffs_unregister(conf.partition_label);
    ESP_LOGI(FILE_SYSTEM_TAG, "SPIFFS unmounted");

    return atoi(barrier_time);
}


