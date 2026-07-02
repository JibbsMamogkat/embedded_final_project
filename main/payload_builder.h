#ifndef PAYLOAD_BUILDER_H
#define PAYLOAD_BUILDER_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define MAX_SLOTS 20

// The structure for our spatial coordinates
struct ParkingSlotROI {
    int id;
    int x;
    int y;
    int w;
    int h;
};

// Global State
ParkingSlotROI active_slots[MAX_SLOTS];
int active_slots_count = 0;

// MVP HARDCODED FALLBACK (Used if Firebase fetch fails)
void load_fallback_coordinates() {
    active_slots_count = 8;
    // Dummy coordinates based on the SM Ecoland test frame
    active_slots[0] = (ParkingSlotROI){1, 10, 20, 100, 100};
    active_slots[1] = (ParkingSlotROI){2, 120, 20, 100, 100};
    active_slots[2] = (ParkingSlotROI){3, 230, 20, 100, 100};
    active_slots[3] = (ParkingSlotROI){4, 340, 20, 100, 100};
    active_slots[4] = (ParkingSlotROI){5, 10, 150, 100, 100};
    active_slots[5] = (ParkingSlotROI){6, 120, 150, 100, 100};
    active_slots[6] = (ParkingSlotROI){7, 230, 150, 100, 100};
    active_slots[7] = (ParkingSlotROI){8, 340, 150, 100, 100};
    printf("Loaded 8 hardcoded fallback coordinates.\n");
}

// ------------------------------------------------------------------
// CORE FUNCTION: Your teammate calls this inside his MQTT loop
// Example usage in main.cpp: 
// char mqtt_msg[256];
// generate_telemetry_payload(mqtt_msg, sizeof(mqtt_msg));
// esp_mqtt_client_publish(client, "parking/ecoland", mqtt_msg, 0, 1, 0);
// ------------------------------------------------------------------
void generate_telemetry_payload(char* json_buffer, size_t buffer_size) {
    if (active_slots_count == 0) {
        load_fallback_coordinates();
    }

    int results[MAX_SLOTS];

    // 1. The Processing Loop
    for(int i = 0; i < active_slots_count; i++) {
        
        // --- TEAMMATE TO UNCOMMENT THIS ON MONDAY ---
        // extract_roi_patch(fb->buf, fb->width, active_slots[i].x, active_slots[i].y, active_slots[i].w, active_slots[i].h, output_patch);
        // preprocess_and_quantize(output_patch, interpreter.input(0)->data.int8);
        // interpreter.Invoke();
        // int8_t occupied = interpreter.output(0)->data.int8[0];
        // int8_t vacant = interpreter.output(0)->data.int8[1];
        // results[i] = (occupied > vacant) ? 1 : 0;
        // --------------------------------------------
        
        // MVP SIMULATION: Randomize 1 or 0 for local testing
        results[i] = (i % 2 == 0) ? 1 : 0; 
    }

    // 2. Dynamic JSON String Builder (Zero Memory Allocation overhead)
    int offset = snprintf(json_buffer, buffer_size, "{\"active_slots\": %d, \"slots\": [", active_slots_count);
    
    for(int i = 0; i < active_slots_count; i++) {
        // Append each result, add a comma if it's not the last item
        offset += snprintf(json_buffer + offset, buffer_size - offset, "%d%s", 
                           results[i], (i == active_slots_count - 1) ? "" : ",");
    }
    
    // Close the JSON array
    snprintf(json_buffer + offset, buffer_size - offset, "]}");
}

#endif // PAYLOAD_BUILDER_H