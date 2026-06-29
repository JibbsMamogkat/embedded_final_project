#include "roi_cropper.h"

void extract_roi_patch(const uint8_t* full_frame, 
                       int frame_width, 
                       int x, int y, int width, int height, 
                       uint8_t* output_patch_64x64) {
    
    // Process the fixed 64x64 output destination grid row by row
    for (int out_y = 0; out_y < 64; ++out_y) {
        // Map destination row back to the source bounding box height space
        int src_y = y + ((out_y * height) / 64);
        
        // Compute the flat row-offset index for the source image to optimize cache reads
        int src_row_offset = src_y * frame_width;
        // Compute the flat row-offset index for our local 64-element destination rows
        int out_row_offset = out_y * 64;

        for (int out_x = 0; out_x < 64; ++out_x) {
            // Map destination column back to the source bounding box width space
            int src_x = x + ((out_x * width) / 64);
            
            // Extract the byte via 1D flat index calculation
            int full_frame_index = src_row_offset + src_x;
            int patch_index = out_row_offset + out_x;
            
            // Assign pixel byte directly into the target execution buffer
            output_patch_64x64[patch_index] = full_frame[full_frame_index];
        }
    }
}