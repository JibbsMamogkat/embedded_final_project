#include "roi_cropper.h"

void extract_roi_patch(const uint8_t* full_frame, 
                       int frame_width, 
                       int x, int y, int width, int height, 
                       uint8_t* output_patch_64x64_rgb) { // Signature updated
    
    for (int out_y = 0; out_y < 64; ++out_y) {
        int src_y = y + ((out_y * height) / 64);
        int src_row_offset = src_y * frame_width;
        int out_row_offset = out_y * 64;

        for (int out_x = 0; out_x < 64; ++out_x) {
            int src_x = x + ((out_x * width) / 64);
            
            // Calculate the exact starting memory address for the 3-byte pixel group
            int full_frame_pixel_idx = (src_row_offset + src_x) * 3;
            int patch_pixel_idx = (out_row_offset + out_x) * 3;
            
            // Body updated to use the matching variable name
            output_patch_64x64_rgb[patch_pixel_idx + 0] = full_frame[full_frame_pixel_idx + 0]; // Red
            output_patch_64x64_rgb[patch_pixel_idx + 1] = full_frame[full_frame_pixel_idx + 1]; // Green
            output_patch_64x64_rgb[patch_pixel_idx + 2] = full_frame[full_frame_pixel_idx + 2]; // Blue
        }
    }
}