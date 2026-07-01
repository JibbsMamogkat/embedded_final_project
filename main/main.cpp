#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_heap_caps.h" // For raw hardware PSRAM access
#include "esp_timer.h"

// Project Core Component Headers
#include "roi_cropper.h"
#include "image_data.h"
#include "model_data.h"
#include "quantization_bridge.h"

// TensorFlow Lite Micro Core API Stack Headers
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

static const char* TAG = "AI_ENGINE";

// Size footprint determined for typical MobileNetV2 operation (1MB *changed)
#define TFLITE_ARENA_SIZE_BYTES (1024 * 1024)

// Intermediate memory block for storing the cropped visual output patch
static uint8_t raw_cropped_patch_rgb[64 * 64 * 3];

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Initializing TinyML Parking Space Pipeline...");

    // ------------------------------------------------------------------------
    // STEP 1: Dynamically Allocate Tensor Arena in Fast External PSRAM
    // ------------------------------------------------------------------------
    uint8_t* tensor_arena = (uint8_t*)heap_caps_malloc(TFLITE_ARENA_SIZE_BYTES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (tensor_arena == nullptr) {
        ESP_LOGE(TAG, "Fatal Error: Unable to allocate tensor arena space from 8MB PSRAM bank!");
        return;
    }
    ESP_LOGI(TAG, "✓ Working Calculation Arena mapped smoothly to PSRAM space.");

    // ------------------------------------------------------------------------
    // STEP 2: Map the Raw Binary Model Array Layout
    // ------------------------------------------------------------------------
    const tflite::Model* model = tflite::GetModel(parking_classifier_model_start);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        ESP_LOGE(TAG, "Fatal Error: Model schema version mismatch! Expected %d, got %d.", 
                 TFLITE_SCHEMA_VERSION, (int)model->version());
        return;
    }
    ESP_LOGI(TAG, "✓ Edge Model flatbuffer signature validated.");

    // ------------------------------------------------------------------------
    // STEP 3: Register Explicit Hardware Vector Operators (Optimized)
    // ------------------------------------------------------------------------
    // Increased slot array capacity to 15 to accommodate the new layers cleanly
    static tflite::MicroMutableOpResolver<15> ops_resolver;
    
    // Register the exact structural layers utilized by MobileNetV2 architectures
    ops_resolver.AddConv2D();
    ops_resolver.AddDepthwiseConv2D();
    ops_resolver.AddFullyConnected();
    ops_resolver.AddAveragePool2D();
    ops_resolver.AddAdd(); // Used for residual skip-connections
    ops_resolver.AddSoftmax();
    ops_resolver.AddReshape();
    ops_resolver.AddQuantize();
    ops_resolver.AddDequantize();
    
    // Fixes the missing opcode registration error!
    ops_resolver.AddPad();

    // Fixes the missing opcode 'MEAN' registration error!
    ops_resolver.AddMean();

    // ------------------------------------------------------------------------
    // STEP 4: Instantiate and Initialize the Interpreter Runtime
    // ------------------------------------------------------------------------
    tflite::MicroInterpreter interpreter(model, ops_resolver, tensor_arena, TFLITE_ARENA_SIZE_BYTES);
    
    if (interpreter.AllocateTensors() != kTfLiteOk) {
        ESP_LOGE(TAG, "Fatal Error: Allocation of active network nodes failed!");
        return;
    }
    ESP_LOGI(TAG, "✓ Dynamic layer tensors allocated safely within arena limits.");

    // Access pointers to internal model input/output channels
    TfLiteTensor* input_tensor = interpreter.input(0);
    TfLiteTensor* output_tensor = interpreter.output(0);

    // ------------------------------------------------------------------------
    // STEP 5: Run C++ Image Cropper Over the Raw Davao Matrix Frame
    // ------------------------------------------------------------------------
    ESP_LOGI(TAG, "Slicing parking boundary targets out of raw Davao image frame data...");
    
    // Simulating full frame coordinates layout mapping 
    int roi_x = 0;
    int roi_y = 0;
    int roi_w = OCCUPIED_WIDTH;
    int roi_h = OCCUPIED_HEIGHT;

    extract_roi_patch(davao_vacant_frame, OCCUPIED_WIDTH, roi_x, roi_y, roi_w, roi_h, raw_cropped_patch_rgb);

    // ------------------------------------------------------------------------
    // STEP 6: Apply ImageNet Normalization and Quantize to INT8 Data Layout
    // ------------------------------------------------------------------------
    ESP_LOGI(TAG, "Passing raw RGB bytes through quantization alignment bridge...");
    
    // Direct pointer mapping to the interpreter's raw internal input memory address
    int8_t* model_input_buffer = input_tensor->data.int8;
    
    preprocess_and_quantize(raw_cropped_patch_rgb, model_input_buffer);

    // ------------------------------------------------------------------------
    // STEP 7: Fire Off Hardware-Accelerated Inference Core Pass
    // ------------------------------------------------------------------------
    ESP_LOGI(TAG, "Invoking MobileNetV2 network forward execution engine...");
    
    int64_t start_ticks = esp_timer_get_time();
    TfLiteStatus invoke_status = interpreter.Invoke();
    int64_t end_ticks = esp_timer_get_time();

    if (invoke_status != kTfLiteOk) {
        ESP_LOGE(TAG, "Error: Neural network forward inference invocation failed!");
        return;
    }

    // Calculate latency metrics in microseconds down to milliseconds
    float execution_time_ms = (float)(end_ticks - start_ticks) / 1000.0f;
    ESP_LOGI(TAG, "✓ Inference execution completed in %.2f ms!", execution_time_ms);

    // ------------------------------------------------------------------------
    // STEP 8: Read Output Logit Tensors and Display Classification Results
    // ------------------------------------------------------------------------
    int8_t* model_output_scores = output_tensor->data.int8;
    int8_t occupied_score = model_output_scores[0]; // Index 0: Occupied
    int8_t vacant_score   = model_output_scores[1]; // Index 1: Vacant

    printf("\n======================================================\n");
    printf("              TINYML INFERENCE RESULTS                \n");
    printf("======================================================\n");
    printf("Raw INT8 Quantized Output Array -> Occupied: [%d], Vacant: [%d]\n", occupied_score, vacant_score);
    
    printf("\nFinal Space Prediction: ");
    if (occupied_score > vacant_score) {
        printf("🔴 OCCUPIED (Vehicle Detected)\n");
    } else {
        printf("🟢 VACANT (Space Open)\n");
    }
    printf("======================================================\n\n");

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}