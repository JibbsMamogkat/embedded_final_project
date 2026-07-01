import cv2
import numpy as np
import os

# 1. Load a sample parking lot image
image_path = "test_frame.jpg" # Change this to your exact image name
if not os.path.exists(image_path):
    print(f"Error: Could not find {image_path}")
    exit()

img = cv2.imread(image_path)

# 2. Resize to VGA resolution and convert to Grayscale (1 byte per pixel)
FRAME_WIDTH = 640
FRAME_HEIGHT = 480
img_resized = cv2.resize(img, (FRAME_WIDTH, FRAME_HEIGHT))
gray_img = cv2.cvtColor(img_resized, cv2.COLOR_BGR2GRAY)

# 3. Flatten into a 1D array
flat_bytes = gray_img.flatten()

# 4. Generate the C++ header file
output_path = "image_data.h"
with open(output_path, "w") as f:
    f.write("#ifndef IMAGE_DATA_H\n#define IMAGE_DATA_H\n\n")
    f.write("#include <stdint.h>\n\n")
    f.write(f"#define DISPLAY_FRAME_WIDTH {FRAME_WIDTH}\n")
    f.write(f"#define DISPLAY_FRAME_HEIGHT {FRAME_HEIGHT}\n\n")
    
    f.write("const uint8_t raw_parking_frame[] = {\n")
    
    # Write bytes in chunks of 12 for clean C++ formatting
    for i in range(0, len(flat_bytes), 12):
        chunk = flat_bytes[i:i+12]
        hex_strings = [f"0x{b:02X}" for b in chunk]
        f.write("    " + ", ".join(hex_strings) + ",\n")
        
    f.write("};\n\n")
    f.write("#endif // IMAGE_DATA_H\n")

print(f"Successfully generated {output_path} with {len(flat_bytes)} bytes!")