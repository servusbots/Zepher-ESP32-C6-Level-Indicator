# 🎯 Zephyr RTOS on Seeed Studio XIAO ESP32C6: Bulls-Eye Digital Level Indicator & Learning Workspace

[![Zephyr RTOS](https://img.shields.io/badge/Zephyr_RTOS-v4.5.0-brightgreen.svg?logo=zephyrproject)](https://zephyrproject.org/)
[![Target MCU](https://img.shields.io/badge/MCU-Seeed_Studio_XIAO_ESP32C6-blue.svg?logo=espressif)](https://www.seeedstudio.com/Seeed-Studio-XIAO-ESP32C6-p-5884.html)
[![Architecture](https://img.shields.io/badge/Arch-RISC--V_32--bit_HP_Core-orange.svg)](https://riscv.org/)
[![Sensors](https://img.shields.io/badge/Sensors-MPU6050_IMU_%2B_SSD1306_OLED-purple.svg)](file:///levelindicator)
[![License](https://img.shields.io/badge/License-Apache--2.0-yellow.svg)](https://opensource.org/licenses/Apache-2.0)

Welcome to the companion repository for the **Seeed Studio XIAO ESP32C6** Zephyr RTOS project! This repository contains a production-grade **Bulls-Eye Digital Level Indicator** application, an interactive YouTube video presentation dashboard, and a progressive 8-lab curriculum for mastering embedded systems development with Zephyr RTOS on RISC-V hardware.

---

## 📺 YouTube Video & Interactive Showcase

* **🎬 Video Script & Production Guide**: [YOUTUBE_VIDEO_GUIDE.md](file:///YOUTUBE_VIDEO_GUIDE.md) — Complete video outline, timestamped chapters, narration talking points, and OBS recording tips.
* **🌐 Interactive Visualizer & Presentation Dashboard**: [showcase.html](file:///showcase.html) — Double-click to open in any web browser! Features a real-time virtual level simulator (mouse tilt), 5-second calibration timer, interactive pinout visualizer, and slide deck for video recording.

---

## 🌟 Spotlight Project: Bulls-Eye Digital Level Indicator (`levelindicator/`)

The flagship application in this repository turns the XIAO ESP32C6, an **MPU-6050** 6-axis IMU, and a **0.96" SSD1306 OLED** display into an ultra-responsive, precision digital surface level.

```
       +-------------------------------------------------------------+
       | [XIAO ESP32C6]                                              |
       |  32-bit RISC-V Core @ 160MHz                                |
       |    │                                                        |
       |    ├─── I2C Bus (Fast-Mode 400kHz, GPIO 22 / 23)            |
       |    │      │                                                 |
       |    │      ├─── MPU-6050 (0x68): 3-Axis Accel Sampling       |
       |    │      └─── SSD1306  (0x3C): 128x64 Monochrome OLED      |
       |    │                                                        |
       |    └─── GPIO 15 (Active-Low Yellow User LED)                |
       +-------------------------------------------------------------+
```

### Key Engineering Features

1. **⏱️ 5-Second Automatic Boot Calibration**:
   - On power-up or system reset, the system enters an active calibration state for exactly 5.0 seconds.
   - Computes reference zero-bias offsets ($\text{bias}_X, \text{bias}_Y$) for whatever surface the device rests upon.
   - Renders a live countdown timer (`Ready in: X.Xs`) and animated progress bar on the OLED display.

2. **🎯 Circular Bulls-Eye Reticle**:
   - Centered target reticle at $(X_0 = 64, Y_0 = 32)$ on the 128x64 display.
   - Concentric reference rings:
     - **Center Bullseye Ring** ($R = 5\text{ px}$): Target level zone ($|\text{angle}| < 1.0^\circ$).
     - **Middle Reference Ring** ($R = 14\text{ px}$): Moderate tilt zone ($\sim 5^\circ$).
     - **Outer Reference Ring** ($R = 24\text{ px}$): High tilt zone ($\sim 10^\circ$).
   - Precision alignment crosshairs with bullseye central clearance.

3. **💧 Dynamic Filtered Bubble**:
   - Smoothly tracks tilt using Euler pitch and roll calculated from accelerometer gravity vectors.
   - Exponential Moving Average (EMA) filtering ($\alpha = 0.30$) eliminates motor vibration and hand jitter.
   - On-screen telemetry readout displaying live angles: `R: +0.0°` (Roll) and `P: -0.0°` (Pitch).

4. **⚡ High-Frequency Screen Inversion Strobe**:
   - When both pitch and roll enter the target bullseye ($< 1.0^\circ$), the display triggers **LEVEL STROBE MODE**:
     - The entire 128x64 screen strobes via hardware inversion at **3.3 Hz** (150 ms period).
     - Bold `*LEVEL*` indicator appears on screen.
     - The onboard yellow LED (`GPIO 15`) pulses in lockstep.

5. **📊 USB-Serial-JTAG Telemetry**:
   - Live angles, calibration states, and loop execution cycle counts stream over native USB (`COM18`) at 115200 baud.

---

## 🔌 Hardware Pinout & Wiring

Both I2C peripherals share the hardware `i2c0` bus on the Seeed Studio XIAO ESP32C6:

```
                  +---[ USB-C ]---+
                  |               |
     (GPIO 0)  D0 | [ ]       [ ] | 5V   (USB 5V In / VBUS)
     (GPIO 1)  D1 | [ ]       [ ] | GND  (Ground) -------------------+-- [GND MPU6050 & OLED]
     (GPIO 2)  D2 | [ ]       [ ] | 3V3  (3.3V Regulated Output) ----+-- [VCC MPU6050 & OLED]
    (GPIO 21)  D3 | [ ]       [ ] | D10  (GPIO 18 / SPI MOSI)
(SDA / GPIO 22) D4 | [ ]       [ ] | D9   (GPIO 20 / SPI MISO)
(SCL / GPIO 23) D5 | [ ]       [ ] | D8   (GPIO 19 / SPI SCK)
 (TX / GPIO 16) D6 | [ ]       [ ] | D7   (GPIO 17 / UART0 RX)
                  |    [R] [B]    |
                  +---------------+
                     Reset Boot
```

### Wiring Table

| Peripheral | Pin | XIAO ESP32C6 Pin | Function | Notes |
|:---|:---|:---|:---|:---|
| **Power** | VCC | **3V3** | 3.3V Power | Powers both MPU-6050 & SSD1306 |
| **Power** | GND | **GND** | Ground | Common reference ground |
| **I2C Bus** | SDA | **D4 (GPIO 22)** | I2C Data | Shared bus, internal pull-up enabled |
| **I2C Bus** | SCL | **D5 (GPIO 23)** | I2C Clock | Shared bus, 400 kHz Fast-Mode |
| **Onboard** | LED | **GPIO 15** | Status LED | Yellow User LED (Active-LOW) |

### I2C Bus Addresses
* **SSD1306 OLED**: `0x3C`
* **MPU-6050 IMU**: `0x68` (AD0 tied to GND / default)

---

## 📐 Mathematical Derivation

Pitch ($\theta$) and Roll ($\phi$) are derived from the 3-axis accelerometer vector $\mathbf{a} = [a_x, a_y, a_z]^T$:

$$\text{Roll} = \text{atan2}(a_y', a_z) \times \frac{180.0}{\pi}$$

$$\text{Pitch} = \text{atan2}(-a_x', \sqrt{a_y'^2 + a_z^2}) \times \frac{180.0}{\pi}$$

Where zero-offset calibration biases are subtracted before angle projection:

$$a_x' = a_x - \text{bias}_x, \quad a_y' = a_y - \text{bias}_y$$

Bubble pixel displacement on the 128x64 display (scale factor $K = 2.2\text{ px/deg}$):

$$X_{\text{bubble}} = 64 + \text{clamp}(K \cdot \text{Roll}, -26, +26)$$

$$Y_{\text{bubble}} = 32 - \text{clamp}(K \cdot \text{Pitch}, -26, +26)$$

---

## ⚡ Quick Start: Build, Flash & Run

### Prerequisites
* Windows 10/11 with PowerShell
* Python 3.10+ in `.venv` with `west`, `esptool`, `pyserial`
* Zephyr SDK 1.0.1 (with `riscv64-zephyr-elf` GCC 14.3.0)

### Step 1: Activate Environment
```powershell
. .\scripts\env.ps1
```

### Step 2: Build the Level Indicator with Sysbuild
Zephyr's `sysbuild` automatically coordinates building the **MCUboot** bootloader and the application binary:
```powershell
west build -p always -b xiao_esp32c6/esp32c6/hpcore --sysbuild .\levelindicator
```

### Step 3: Flash to XIAO ESP32C6 (COM18)
Our custom flashing script detects sysbuild domains and programs both MCUboot (`0x0`) and the application image (`0x20000`):
```powershell
.\scripts\flash.ps1 -Port COM18
```
> [!IMPORTANT]
> Because the ESP32-C6 USB-Serial-JTAG controller holds the chip in ROM download mode after flashing (`boot:0x16`), press the **'R' (Reset)** button on the board once to start firmware execution!

### Step 4: Monitor Serial Telemetry
```powershell
.\scripts\monitor.ps1 -Port COM18
```

---

## 📚 Complete 8-Lab Zephyr Curriculum

In addition to the Bulls-Eye Level Indicator, this repository contains 8 standalone, self-documenting labs located in `labs/`:

| Lab | Directory | Topic | Key Hardware / Concept |
|:---:|:---|:---|:---|
| **01** | [`labs/01_blinky`](file:///labs/01_blinky) | GPIO & Devicetree Basics | Onboard Yellow LED (`GPIO 15`), Active-Low Polarity |
| **02** | [`labs/02_button_interrupt`](file:///labs/02_button_interrupt) | Hardware Interrupts (ISR) | External push button on `D1 (GPIO 1)` with debouncing |
| **03** | [`labs/03_usb_logging_and_shell`](file:///labs/03_usb_logging_and_shell) | USB CDC & Zephyr Shell | Interactive command-line shell over USB Serial |
| **04** | [`labs/04_rtos_multithreading`](file:///labs/04_rtos_multithreading) | RTOS Preemptive Threads | Static threads, semaphores, and message queues |
| **05** | [`labs/05_timers_and_workqueues`](file:///labs/05_timers_and_workqueues) | Timers & System Workqueues | Software `k_timer` and bottom-half deferral |
| **06** | [`labs/06_pwm_fade`](file:///labs/06_pwm_fade) | Hardware PWM (LEDC) | Hardware LED breathing effect using Devicetree overlays |
| **07** | [`labs/07_i2c_scanner`](file:///labs/07_i2c_scanner) | I2C Driver Subsystem | Linux `i2cdetect` matrix scanning addresses `0x01`–`0x7F` |
| **08** | [`labs/08_ble_beacon`](file:///labs/08_ble_beacon) | Bluetooth 5.3 Low Energy | GAP non-connectable telemetry beacon broadcasting |

---

## 🛠️ Repository File Structure

```text
SeedStudio_c6_test/
├── levelindicator/               # 🎯 Main Bulls-Eye Level Indicator Application
│   ├── CMakeLists.txt            # Application build configuration
│   ├── prj.conf                  # Kconfig symbols (Sensor, Display, I2C, Float)
│   ├── app.overlay               # Devicetree: SSD1306 (0x3C) & MPU6050 (0x68) on i2c0
│   ├── README.md                 # Detailed engineering notes for the app
│   └── src/
│       ├── main.c                # 5s calibration, math, graphics engine, main loop
│       ├── font5x7.h             # 5x7 ASCII bitmap font header
│       └── font5x7.c             # Compact font character bitmasks
├── labs/                         # 📚 8 Progressive Zephyr RTOS Hands-On Labs
│   ├── 01_blinky/
│   ├── 02_button_interrupt/
│   ├── 03_usb_logging_and_shell/
│   ├── 04_rtos_multithreading/
│   ├── 05_timers_and_workqueues/
│   ├── 06_pwm_fade/
│   ├── 07_i2c_scanner/
│   └── 08_ble_beacon/
├── docs/                         # 📖 Technical Documentation & Reference Guides
│   ├── XIAO_ESP32C6_HARDWARE.md   # Pinout, schematics, clocking, power rails
│   ├── ZEPHYR_ARCHITECTURE_GUIDE.md # Devicetree, Kconfig, Sysbuild deep dive
│   ├── ZEPHYR_CHEATSHEET.md      # Command reference and API cheat sheet
│   └── TROUBLESHOOTING.md        # Hardware, ROM bootloader, and SDK solutions
├── scripts/                      # ⚙️ Automation Scripts
│   ├── env.ps1                   # Environment activation
│   ├── flash.ps1                 # Intelligent sysbuild domain flasher
│   ├── monitor.ps1               # PySerial 115200 baud monitor
│   ├── install_sdk.ps1           # Zephyr SDK 1.0.1 installer & exporter
│   └── setup_workspace.ps1       # Workspace initialization
├── showcase.html                 # 🌐 Interactive YouTube Video Visualizer & Simulator
├── YOUTUBE_VIDEO_GUIDE.md        # 🎬 Complete YouTube Video Script & Production Notes
├── west.yml                      # West manifest pinning Zephyr v4.5 & HALs
└── README.md                     # This file
```

---

## 🤝 Contributing & License

Contributions, bug reports, and lab ideas are welcome! Feel free to open an issue or pull request.

Distributed under the **Apache-2.0 License**. See [LICENSE](LICENSE) for details.
