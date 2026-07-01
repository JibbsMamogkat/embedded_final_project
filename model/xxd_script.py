import os

input_file = "mobilenet_v2_64x64_quantized.tflite"
output_file = "model_data.h"
array_name = "mobilenet_v2_64x64_quantized_tflite"

print(f"Reading {input_file}...")

try:
    with open(input_file, "rb") as f:
        data = f.read()

    print("Formatting binary data to C array...")
    with open(output_file, "w") as f:
        f.write(f"unsigned char {array_name}[] = {{\n")
        
        # Convert bytes to hex format
        hex_array = [f"0x{b:02x}" for b in data]
        
        # Write 12 bytes per line for readability
        for i in range(0, len(hex_array), 12):
            f.write("  " + ", ".join(hex_array[i:i+12]) + ",\n")
            
        f.write("};\n")
        f.write(f"unsigned int {array_name}_len = {len(data)};\n")
        
    print(f"Success! Saved to {output_file}")

except FileNotFoundError:
    print(f"Error: Make sure {input_file} is in the same folder as this script.")