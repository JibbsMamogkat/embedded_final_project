#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "roi_cropper.h"
#include "image_data.h" // Links your raw color dataset array

// Allocate a 3-channel RGB target array buffer (64 * 64 * 3 = 12,288 bytes)
static uint8_t output_patch_rgb[64 * 64 * 3];

extern "C" void app_main(void) {
    ESP_LOGI("PIPELINE", "Starting Hardware-in-the-Loop Static Emulation Test...");
    ESP_LOGI("PIPELINE", "Loaded Target Frame Dimensions: %dx%d (RGB Channels)", OCCUPIED_WIDTH, OCCUPIED_HEIGHT);

    // Target the entire boundaries of the pre-cropped Davao space photo
    int roi_x = 0;
    int roi_y = 0;
    int roi_w = OCCUPIED_WIDTH;
    int roi_h = OCCUPIED_HEIGHT;

    ESP_LOGI("PIPELINE", "Executing 3-channel spatial downscale to 64x64x3 tensor matrix...");

    // Run processing math directly over the real davao array asset memory location
    extract_roi_patch(davao_occupied_frame, OCCUPIED_WIDTH, roi_x, roi_y, roi_w, roi_h, output_patch_rgb);

    ESP_LOGI("PIPELINE", "Array matrix operation completed successfully!");
    
    // Print a 4x4 matrix block showing the true R,G,B integer triplets per pixel coordinate
    printf("\nSample RGB Output Matrix (Top-Left Pixel Blocks):\n");
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            int idx = (y * 64 + x) * 3;
            printf("[%03d,%03d,%03d] ", output_patch_rgb[idx + 0],  // Red
                                        output_patch_rgb[idx + 1],  // Green
                                        output_patch_rgb[idx + 2]); // Blue
        }
        printf("\n");
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}