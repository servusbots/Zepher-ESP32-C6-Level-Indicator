# 🎬 YouTube Video Production Guide: Building a Digital Bulls-Eye Level Indicator with Zephyr RTOS & ESP32-C6

This guide is your complete production companion for filming and publishing a high-engagement YouTube video covering the **Seeed Studio XIAO ESP32C6 Bulls-Eye Digital Level Indicator** built with **Zephyr RTOS**.

---

## 📌 Video Metadata

### 💡 Video Title Options
* **Option 1 (High CTR / Curiosity)**: *I Built a Precision Bulls-Eye Digital Level with Zephyr RTOS & RISC-V ESP32-C6!*
* **Option 2 (Tutorial / Direct)**: *Zephyr RTOS Tutorial: MPU6050 IMU + OLED Bulls-Eye Level on Seeed Studio XIAO ESP32C6*
* **Option 3 (Pro Dev / Enterprise)**: *Ditching Arduino for Zephyr RTOS: Real-Time IMU Level Indicator on ESP32-C6 (RISC-V)*

### 📝 YouTube Video Description (Copy & Paste Ready)
```text
In this video, we build a precision, real-time Bulls-Eye Digital Level Indicator on the Seeed Studio XIAO ESP32-C6 using Zephyr RTOS! 

We wire up an MPU-6050 6-axis IMU and a 0.96" SSD1306 monochrome OLED display over a shared 400kHz I2C bus, write a custom Devicetree overlay, implement a 5-second zero-offset boot calibration, build a lightweight 1KB graphics engine from scratch, and add high-frequency screen strobing when the device hits dead level (<1.0°).

⭐ All source code, labs, and interactive presentation dashboard:
https://github.com/servusbots/Zepher-ESP32-C6-Level-Indicator

⏱️ TIMESTAMPS:
00:00 - Teaser & Live Demonstration (Tilting the Level!)
01:10 - Why Zephyr RTOS over Arduino on ESP32-C6?
02:45 - Hardware Breakdown & I2C Wiring (XIAO + MPU6050 + SSD1306)
05:15 - Zephyr Devicetree & Kconfig Magic (app.overlay & prj.conf)
08:20 - Sensor Mathematics: Euler Angles, 5s Calibration & EMA Filter
11:30 - Writing a 1024-Byte Monochrome Graphics Engine in C
14:15 - Building & Flashing with Sysbuild & MCUboot on COM18
17:00 - Live Testing: Calibration, Tilting, and Level Strobe!
19:30 - Summary & Next Steps (Wi-Fi 6 & BLE Telemetry)

🔧 HARDWARE USED:
• Seeed Studio XIAO ESP32C6 (RISC-V 32-bit SoC)
• MPU-6050 6-Axis Accelerometer / Gyroscope Module
• 0.96" I2C Monochrome OLED Display (SSD1306, 128x64)
• Half-size breadboard & jumper wires
• USB Type-C Cable

💻 SOFTWARE & TOOLCHAIN:
• Zephyr RTOS v4.5.0
• Zephyr SDK 1.0.1 (riscv64-zephyr-elf GCC 14.3.0)
• West Meta-Tool & Python Virtual Environment
• Visual Studio Code + PowerShell

#ZephyrRTOS #ESP32C6 #EmbeddedSystems #RISCV #IoT #Maker #MPU6050 #OLED #Electronics
```

### 🖼️ Thumbnail Blueprint
* **Left Side**: Close-up macro photo of the breadboard with the XIAO ESP32C6 and the lit 0.96" OLED showing the circular bulls-eye reticle with the bubble centered and `*LEVEL*` glowing.
* **Right Side**: Bold 3-line text with glowing border: **"ZEPHYR RTOS"** (Zephyr green) / **"ESP32-C6"** (white) / **"BUBBLE LEVEL"** (electric cyan).
* **Center / Accent**: Small RISC-V badge icon and Zephyr kite logo in upper corner.

---

## 🎥 Scene-by-Scene Script & Recording Blueprint

### Scene 1: The Hook (00:00 - 01:10)
* **Visual**: Close-up top-down camera on the breadboard. Hand tilts the board:
  * The bubble glides smoothly across concentric target rings.
  * As the board levels flat on the desk, the OLED screen **flashes vigorously** at 3.3 Hz and the yellow LED pulses in sync!
* **Narration**:
  > *"Watch this: I tilt the board, and this precision digital bubble glides smoothly across a custom circular bulls-eye reticle. But watch what happens when I set it down completely flat... boom! The screen strobes, the yellow LED locks on, and we're dead level to within less than one single degree.*
  >
  > *Today, we aren't using Arduino. We are building this from bare metal up on the 32-bit RISC-V Seeed Studio XIAO ESP32C6 using an enterprise real-time operating system: **Zephyr RTOS**! Let's dive in!"*

---

### Scene 2: Why Zephyr RTOS on ESP32-C6? (01:10 - 02:45)
* **Visual**: Screen capture showing the project repository and architecture diagram (or open `showcase.html` slide 2).
* **Talking Points**:
  * The ESP32-C6 is Espressif’s RISC-V flagship with Wi-Fi 6, Bluetooth 5.3, and 802.15.4 (Thread/Zigbee).
  * Why Zephyr instead of Arduino or bare ESP-IDF?
    1. **Strict Hardware Abstraction**: Same code can run on Nordic nRF53, STM32, or ESP32 without changing API calls.
    2. **Devicetree**: Hardware pinout and bus configurations live in declarative `.dts` / `.overlay` files, completely separated from C logic.
    3. **Preemptive RTOS**: Rock-solid scheduling, deterministic timers, power management, and industrial sensor drivers baked right into the tree.

---

### Scene 3: Hardware Breakdown & I2C Bus Wiring (02:45 - 05:15)
* **Visual**: Split screen with breadboard camera and clear graphic of the XIAO pinout (from `README.md` or `showcase.html`).
* **Wiring Table Highlight**:
  * **VCC** $\rightarrow$ `3V3` on XIAO (both OLED and MPU6050 share the 3.3V rail).
  * **GND** $\rightarrow$ `GND` on XIAO.
  * **SDA** $\rightarrow$ `D4 (GPIO 22)` on XIAO.
  * **SCL** $\rightarrow$ `D5 (GPIO 23)` on XIAO.
* **Pro-Tip**:
  > *"Both devices share the exact same two I2C wires because they have unique hardware addresses: `0x3C` for the SSD1306 OLED, and `0x68` for the MPU-6050 IMU. The ESP32-C6's internal pull-ups handle the bus lines effortlessly at 400kHz Fast-Mode!"*

---

### Scene 4: Devicetree & Kconfig Architecture (05:15 - 08:20)
* **Visual**: VS Code showing `levelindicator/app.overlay` and `levelindicator/prj.conf`.
* **Explanation of `app.overlay`**:
  ```dts
  &i2c0 {
      status = "okay";
      clock-frequency = <I2C_BITRATE_FAST>; /* 400 kHz */

      ssd1306: ssd1306@3c {
          compatible = "solomon,ssd1306fb";
          reg = <0x3c>;
          width = <128>;
          height = <64>;
          segment-offset = <0>;
          page-offset = <0>;
          display-offset = <0>;
          multiplex-ratio = <63>;
          prechargep = <0x22>;
      };

      mpu6050: mpu6050@68 {
          compatible = "invensense,mpu6050";
          reg = <0x68>;
          status = "okay";
      };
  };
  ```
* **Explanation of `prj.conf`**:
  * `CONFIG_SENSOR=y`, `CONFIG_MPU6050=y` $\rightarrow$ Zephyr compiles the built-in MPU6050 driver.
  * `CONFIG_DISPLAY=y`, `CONFIG_SSD1306=y` $\rightarrow$ Zephyr compiles the SSD1306 frame-buffer driver.
  * `CONFIG_CBPRINTF_FP_SUPPORT=y` $\rightarrow$ Enables floating-point `%f` support in `printk` and logging.

---

### Scene 5: Sensor Math & The 5-Second Boot Calibration (08:20 - 11:30)
* **Visual**: Mathematical slides in `showcase.html` showing Euler angles and vector projection.
* **Talking Points**:
  * **The 5-Second Calibration**: When powered up, what if the user's table isn't 100% true? The device spends the first 5 seconds taking 50 samples, averaging the resting $a_x$ and $a_y$ gravity biases. On screen, a sleek progress bar and countdown timer keeps the user informed: `"Calibrating... Ready in: 3.2s"`.
  * **Euler Angles**:
    $$\text{Roll} = \text{atan2}(a_y', a_z) \times \frac{180}{\pi}$$
    $$\text{Pitch} = \text{atan2}(-a_x', \sqrt{a_y'^2 + a_z^2}) \times \frac{180}{\pi}$$
  * **Exponential Moving Average (EMA)**:
    $$\text{angle}_{\text{filtered}} = \alpha \cdot \text{angle}_{\text{new}} + (1 - \alpha) \cdot \text{angle}_{\text{prev}}$$
    With $\alpha = 0.30$, we get lightning-fast responsiveness without hand tremor or jitter!

---

### Scene 6: The 1024-Byte Graphics Engine in C (11:30 - 14:15)
* **Visual**: VS Code showing `levelindicator/src/main.c` graphics primitives.
* **Code Tour**:
  * SSD1306 uses 128 columns $\times$ 8 vertical pages (8 bits per byte = 1024 bytes total VRAM).
  * We created standalone raster functions:
    * `fb_set_pixel(x, y, color)`: Bitwise manipulation (`vram[page * 128 + x] |= (1 << (y % 8))`).
    * `draw_circle()`: Midpoint Bresenham circle algorithm for the 3 concentric target rings ($R = 5, 14, 24$).
    * `draw_filled_circle()`: Renders the dynamic $R = 3$ bubble.
    * `draw_crosshairs()`: Crosshair reticle with a center cutout for the bullseye.
    * `draw_string_5x7()`: ASCII bitmap text renderer for live telemetry and countdowns.
  * **The Strobe Effect**:
    * When `is_level` is true, a $150\text{ ms}$ timer toggles `flash_invert`. If active, we invert all 1024 bytes (`byte ^= 0xFF`), creating an eye-catching visual flash!

---

### Scene 7: Building & Flashing with Sysbuild & MCUboot (14:15 - 17:00)
* **Visual**: PowerShell terminal running the build and flash commands.
* **Terminal Commands**:
  ```powershell
  # 1. Activate Environment
  . .\scripts\env.ps1

  # 2. Build with sysbuild
  west build -p always -b xiao_esp32c6/esp32c6/hpcore --sysbuild .\levelindicator

  # 3. Flash to board on COM18
  .\scripts\flash.ps1 -Port COM18
  ```
* **Crucial Pro-Tip to share with viewers**:
  > *"Notice that after flashing over the USB-Serial-JTAG port, the ESP32-C6 stays in ROM download mode (`boot:0x16`). You must press the **'R' (Reset)** button on the XIAO board once to kick off normal SPI flash execution. We built an automated watchdog reset into our script to help, but having that button handy is key!"*

---

### Scene 8: Live Testing & Showcase (17:00 - 19:30)
* **Visual**: Full-screen high-definition top-down camera capturing the board.
* **Test Sequence**:
  1. Press 'R' button: show the 5-second calibration progress bar count down from 5.0s to 0.0s.
  2. Tilt the board in X and Y directions: watch the bubble glide smoothly and observe the roll/pitch degrees update in real-time.
  3. Place a spirit level or flat surface underneath: watch the bubble glide into the center $R = 5$ ring.
  4. **The Money Shot**: The instant both angles drop below $1.0^\circ$, the entire screen strobes at 3.3 Hz and `*LEVEL*` appears!
  5. Show serial monitor output on `COM18` logging at 115200 baud.

---

### Scene 9: Outro & Next Steps (19:30 - 21:00)
* **Visual**: GitHub repo page, `README.md`, and lab folders.
* **Call to Action**:
  > *"All of the code, Devicetree overlays, build scripts, and even an interactive HTML presentation simulator are available in our GitHub repository linked in the description below.*
  >
  > *If you're interested in taking this further, check out the 8 foundational labs in the repo—covering everything from BLE 5.3 beacons to hardware PWM and multithreading on the ESP32-C6.*
  >
  > *Don't forget to hit like, subscribe to ServusBots, and drop a comment below on what Zephyr project you want to see next! See you in the next one!"*

---

## 🛠️ Recording Checklist

- [ ] Breadboard wired securely (XIAO ESP32C6 + MPU6050 + SSD1306).
- [ ] USB cable connected to PC on `COM18`.
- [ ] Good overhead lighting on the OLED screen (avoid direct glare/reflection on the glass).
- [ ] Screen recorder set to 1080p60 or 4K.
- [ ] PowerShell terminal font sized up for readability (e.g. 18pt Cascadia Code).
- [ ] Open `showcase.html` in Chrome/Edge on a secondary display or OBS window capture for clean graphic overlays!
