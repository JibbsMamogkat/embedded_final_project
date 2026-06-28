#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "roi_cropper.h"

extern "C" void app_main(void) {
    while (true) {
        ESP_LOGI("SMOKE_TEST", "ESP32-S3 Embedded Environment Online!");
        vTaskDelay(pdMS_TO_TICKS(2000)); // Non-blocking FreeRTOS 2-second sleep
    }
}