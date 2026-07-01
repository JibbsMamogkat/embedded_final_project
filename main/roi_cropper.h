#ifndef ROI_CROPPER_H
#define ROI_CROPPER_H

#include <stdint.h>

// Shared function contract declaration
void extract_roi_patch(const uint8_t* full_frame, 
                       int frame_width, 
                       int x, int y, int width, int height, 
                       uint8_t* output_patch_64x64_rgb);

#endif // ROI_CROPPER_H