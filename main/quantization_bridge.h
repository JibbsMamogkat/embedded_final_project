#ifndef QUANTIZATION_BRIDGE_H
#define QUANTIZATION_BRIDGE_H

#include <stdint.h>
#include <math.h>

// Processes raw RGB configurations directly into the model's target INT8 array layout
inline void preprocess_and_quantize(const uint8_t* raw_cropped_rgb, int8_t* tflite_input_tensor) {
    // Exact ImageNet normalization parameters used in Duff's PyTorch/TFLite scripts
    const float mean[3] = {0.485f, 0.456f, 0.406f};
    const float std_dev[3] = {0.229f, 0.224f, 0.225f};
    
    // Exact structural scales pulled directly from Duff's WebAssembly calibration engine
    const float scale = 0.018658448f;
    const int32_t zero_point = -14;

    for (int i = 0; i < 64 * 64; ++i) {
        for (int c = 0; c < 3; ++c) {
            int flat_idx = (i * 3) + c;

            // 1. Map 0-255 byte ranges back to a raw 0.0-1.0 float boundary
            float pixel_float = (float)raw_cropped_rgb[flat_idx] / 255.0f;

            // 2. Apply explicit ImageNet channel shift offsets
            float normalized = (pixel_float - mean[c]) / std_dev[c];

            // 3. Scale and shift values into the targeted quantized workspace boundaries
            int32_t quantized = (int32_t)roundf(normalized / scale) + zero_point;

            // 4. Clip boundaries strictly to prevent integer overflows
            if (quantized < -128) quantized = -128;
            if (quantized > 127) quantized = 127;

            // Save the aligned int8 sample directly into the model's active memory lane
            tflite_input_tensor[flat_idx] = (int8_t)quantized;
        }
    }
}

#endif // QUANTIZATION_BRIDGE_H