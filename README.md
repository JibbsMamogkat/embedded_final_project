
---

# ESP32-S3 Edge Computer Vision Simulation Workspace

This repository houses the bare-metal firmware execution and spatial preprocessing simulation phase for our computer vision-based parking-slot monitoring system. The entire architecture is built natively upon the **Espressif IoT Development Framework (ESP-IDF)**, targeting the physical **ESP32-S3** microcontroller. 

---

## 🛠️ Detailed Environment Setup & VS Code Integration

To ensure repeatable builds across different development machines, follow this explicit setup pipeline to establish an identical compiler state.

### 1. ESP-IDF Toolchain Core Installation
1. Download the official **ESP-IDF Universal Online Installer (v2.4.1)**.
2. Launch the installer and select **ESP-IDF v5.3.5 (Release Version)** from the checklist. 
   * *Note: Avoid the moving release branches. Choosing the static v5.3.5 release archive provides absolute configuration stability throughout this sprint.*
3. Choose **Full Installation** to ensure all background cross-compilers (`xtensa-esp32s3-elf-g++`), Python virtual environments, flashing tools (`esptool.py`), and OpenOCD debugging assets are linked cleanly.
4. When prompted for destination paths, redirect the default directory fields away from your operating system's drive to your primary secondary storage partition. Keep track of these two paths for the next phase.

### 2. Visual Studio Code Extension Linking
1. Launch VS Code and navigate to the Extensions Marketplace (`Ctrl + Shift + X`).
2. Search for and install the official **ESP-IDF** extension published by **Espressif**.
3. Open the Command Palette (`Ctrl + Shift + P`), type **`ESP-IDF: Configure ESP-IDF Extension`**, and hit Enter.
4. Select the **"USE EXISTING SETUP"** option. The configuration manager will perform a local directory sweep, discover your customized installation paths, map the underlying tool variables, and register the cross-compilers seamlessly.

### 3. Workspace Initialization
1. In VS Code, navigate to `File -> Open Folder...` and select the root directory containing this repository.
2. Look at the lower **Status Bar** at the bottom edge of the window to verify the target state:
   * **Target Chip:** Click the chip selector icon and set it explicitly to **`esp32s3`**.
   * **COM Port:** Connect your physical ESP32-S3 development board using a USB data cable, click the port selector icon, and choose the assigned active communication interface channel.

---

## 🎯 Overall Architecture & Firmware Workflow

Unlike a standard desktop application running within an operating system wrapper, this system executes an optimization-focused, real-time edge processing pipeline driven directly by the microcontroller's core logic.

```text
  [ Raw Frame Data Buffer ]  <-- Simulated Camera DMA Array (image_data.h)
             │
             ▼
  [ Sequential ROI Slicing ] <-- Row-Stride Index Transformation (Role 1)
             │
             ▼
  [ Spatial Normalization ]  <-- Nearest-Neighbor 64x64 Downscaling (Role 1)
             │
             ▼
  [ Quantized Edge Evaluation ] <-- Static INT8 Tensor Arena (Role 2 / model_data.h)
             │
             ▼
   [ UART Telemetry Logs ]   <-- Real-time FreeRTOS Stream (app_main)

```

### The Runtime Execution Sequence:

1. **Memory Staging:** A high-resolution, wide-angle image frame is staged in memory as a raw 1D byte array inside `image_data.h`. This array structure perfectly mirrors a live camera sensor dumping a frame buffer directly into memory via Direct Memory Access (DMA).
2. **Sequential Cropping Loop:** The main execution path iterates through a predefined coordinate array mapping our target parking zones. The system uses specific pointer math to isolate data segments directly out of the primary array block.
3. **Sizing Normalization:** The variable dimensional slices extracted from the main frame buffer are immediately pushed through an interpolation algorithm, downscaling them into uniform $64 \times 64$ byte matrices to accommodate edge memory space limits.
4. **Quantized Classification:** The standardized array patches are evaluated against our target classification engine. The system analyzes the spatial arrays utilizing the operational thresholds derived from our trained machine learning metrics.
5. **Real-Time Telemetry:** The classification states (`OCCUPIED` / `VACANT`) and confidence boundaries for each individual slot ID are packaged and streamed over the UART channel via the native `ESP_LOGI` diagnostic logging framework.

---

## 🧠 The ML Quantization Bridge (Crucial Architecture Note)

To ensure high processing speeds on low-power hardware, our machine learning pipeline utilizes **Post-Training Quantization (PTQ)**. It is critical to understand the distinction between where the model is optimized and where it is executed:

* **Where the Quantization Code Lives:** The code responsible for compressing our MobileNetV2 network sits **entirely within our offline desktop Python/PyTorch repository**, completely decoupled from this firmware codebase. The optimization scripts convert the high-precision 32-bit floating-point weights ($FP32$) into compact, highly optimized 8-bit integers ($INT8$).
* **How the Model Integrates with the C++ Code:** The compiled output of that Python quantization script is a compressed `.tflite` model file. We run a serialization pass on the desktop to convert this binary file into an array of raw hexadecimal bytes stored in a custom header file:
```bash
xxd -i quantized_mobilenet.tflite > main/model_data.h

```


* **Firmware Execution:** The generated `model_data.h` is dropped directly into our `main/` component folder. When `idf.py build` is triggered, the compiler treats this file as a plain `const unsigned char` array, embedding the model weights permanently into the ESP32-S3's internal flash storage. The C++ runtime loop reads those static bytes directly from flash during live evaluations, eliminating the need to compile any Python files on the hardware itself.

---

## 🤝 2-Person Task Distribution Matrix

To facilitate rapid feature iteration and maintain clean version isolation, our collaborative tasks are divided strictly at the component file layer.

### 👥 Development Split

#### Role 1: The Spatial Layout & Pointer Engineer (The Cropper)

* **Component Ownership:** `main/roi_cropper.h` and `main/roi_cropper.cpp`
* **Objective:** Implement all geometric transformation, memory slicing, and spatial adjustment math inside the firmware.
* **Core Deliverables:**
* Build the index tracking math to calculate coordinates directly out of flat 1D byte structures.
* Implement the processing structures to handle individual bounding boxes sequentially.
* Code the downscaling interpolation loop to format variable crops into uniform $64 \times 64$ matrices.



#### Role 2: The Staging & Telemetry Engineer (The Predictor)

* **Component Ownership:** `main/image_data.h`, `main/model_data.h`, and `main/main.cpp`
* **Objective:** Manage dataset preparation, firmware loop orchestration, system resource allocations, and serial communications.
* **Core Deliverables:**
* Script the off-line Python tool to transform raw test images into C++ header files.
* Establish the root application task engine (`app_main`) leveraging FreeRTOS multitasking controls.
* Build the mock inference logic mapping out state configurations and real-time `ESP_LOGI` text streams.



### 📜 The Shared Interface Contract

Both roles must build around the unified function interface declared inside `main/roi_cropper.h`:

```cpp
void extract_roi_patch(const uint8_t* full_frame, 
                       int frame_width, 
                       int x, int y, int width, int height, 
                       uint8_t* output_patch_64x64);

```

* **Role 1** writes the internal logic inside `roi_cropper.cpp` to correctly populate the `output_patch_64x64` memory space.
* **Role 2** calls this function inside the core system execution loop within `main.cpp`.

---

## 🔮 Future-Proofing for Hardware (OV2640 Camera Integration)

When we shift from static mock testing files to the real **OV2640 physical camera module**, the core architectural work done by Role 1 remains fully valid. The official Espressif `esp_camera` driver automatically handles I2C configurations and uses DMA to stream pixel arrays into a custom structure called `camera_fb_t`.

Role 1 must design their spatial logic with the following two hardware constraints in mind to ensure a seamless drop-in transition later:

### 1. Interfacing with the Driver Pointer

Instead of reading from a loose array, the firmware loop will read from the driver's frame buffer struct pointer (`fb->buf`). Role 2 will unpack this structure and forward the data stream into Role 1’s algorithm like this:

```cpp
camera_fb_t* fb = esp_camera_fb_get();
extract_roi_patch(fb->buf, fb->width, x, y, w, h, output_buffer);
esp_camera_fb_return(fb);

```

### 2. The Color-Space Pixel Stride

The coordinate offset calculations rely heavily on the camera format chosen during system configuration:

* **Grayscale Mode (`PIXFORMAT_GRAYSCALE`):** Each pixel maps directly to 1 byte in memory. The spatial index tracking loop matches the standard layout exactly:

$$\text{Index} = (y \times \text{Frame\_Width}) + x$$


* **Color Mode (`PIXFORMAT_RGB565`):** Each pixel spans **2 bytes** (16 bits) to store red, green, and blue attributes. To prevent the cropping loop from tearing or misaligning image slices, Role 1 must include an adjustable pixel stride multiplier (`bytes_per_pixel = 2`):

$$\text{Byte Index} = 2 \times ((y \times \text{Frame\_Width}) + x)$$



---

## 🌲 Collaborator Branching & Git Workflow

To prevent merge conflicts and avoid corrupting shared target settings (such as the auto-generated `sdkconfig` file), direct pushes to the `main` branch are restricted. Follow this branch development protocol:

1. Prior to commencing any development cycle, pull down the latest verified foundation code:
```bash
git checkout main
git pull origin main

```


2. Spawn a dedicated, purpose-built feature branch for your tasks:
```bash
git checkout -b feature/spatial-cropper        # (Role 1 Branch)
git checkout -b feature/telemetry-predictor    # (Role 2 Branch)

```


3. Implement your changes exclusively within your designated files, test the modifications locally, and push the branch to the remote repository:
```bash
git add .
git commit -m "feat: complete initial patch extraction milestone"
git push origin feature/your-branch-name

```


4. Navigate to the GitHub repository page and open a **Pull Request (PR)**. Review the changes together to ensure code compliance before merging the branch into `main`.

---

## 🚀 How to Build, Flash, and Monitor

All operations should be conducted via the **ESP-IDF PowerShell Environment** or the automated tool icons built into the VS Code status bar.

From your project directory root, execute the compilation and deployment sequence:

```bash
# Verify the target compiler layout is configured for the ESP32-S3 architecture
idf.py set-target esp32s3

# Build the source, push the binary via the COM port, and open the active monitoring window
idf.py build flash monitor

```

### Essential Terminal Shortcuts:

* **Terminate the active Serial Monitor interface:** Press `Ctrl + ]`
* **Trigger a hardware system reset via terminal:** Press `Ctrl + T`, then press `Ctrl + R`

```

```