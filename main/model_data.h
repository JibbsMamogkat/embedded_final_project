#ifndef MODEL_DATA_H
#define MODEL_DATA_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Linker generated symbols marking the raw flash boundaries of the embedded file
extern const uint8_t _binary_mobilenet_v2_64x64_quantized_tflite_start[] __attribute__((aligned(16)));
extern const uint8_t _binary_mobilenet_v2_64x64_quantized_tflite_end[];

// Direct macro alias so your existing main.cpp pipeline code doesn't break
#define parking_classifier_model_start _binary_mobilenet_v2_64x64_quantized_tflite_start

// Dynamically calculates the exact byte array size using clean pointer arithmetic
#define CLASSIFIER_MODEL_SIZE_BYTES (_binary_mobilenet_v2_64x64_quantized_tflite_end - _binary_mobilenet_v2_64x64_quantized_tflite_start)

#ifdef __cplusplus
}
#endif

#endif // MODEL_DATA_H