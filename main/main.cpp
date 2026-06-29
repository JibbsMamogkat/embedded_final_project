#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "roi_cropper.h"

// Allocation arrays representing our image target grids
static uint8_t mock_full_frame[320 * 240]; // Mocking a standard QVGA resolution array frame
static uint8_t output_patch[64 * 64];      // Target model input tensor buffer

extern "C" void app_main(void) {
    // Fill the mock frame with simple sequential tracking values
    for (int i = 0; i < (320 * 240); ++i) {
        mock_full_frame[i] = (uint8_t)(i % 256);
    }

    // Bounding Box Test Coordinates (Simulating a detected vehicle box layout)
    int roi_x = 40;
    int roi_y = 30;
    int roi_w = 120;
    int roi_h = 90;

    ESP_LOGI("PIPELINE", "Executing spatial downscale to 64x64 tensor matrix...");

    // Run the extraction process
    extract_roi_patch(mock_full_frame, 320, roi_x, roi_y, roi_w, roi_h, output_patch);

    ESP_LOGI("PIPELINE", "Array processing pass completed successfully!");
    
    // Print out a tiny 4x4 matrix sample corner of our output array to check values
    printf("Sample Array Output (Top Left Corner):\n");
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            printf("%03d ", output_patch[y * 64 + x]);
        }
        printf("\n");
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}