# Zephyr RTOS Learning Workspace: Seeed Studio XIAO ESP32C6

Welcome to your dedicated **Zephyr RTOS** learning environment crafted specifically for the **Seeed Studio XIAO ESP32C6** microcontroller!

This workspace is designed to take you from initial setup through core RTOS concepts, hardware drivers, multithreading, and wireless communication using hands-on labs, automated scripts, and deep documentation.

---

## 🧭 Hardware Overview: Seeed Studio XIAO ESP32C6

The **XIAO ESP32C6** packs a high-performance 32-bit RISC-V SoC into a thumb-sized 14-pin form factor:

```text
                  +---[ USB-C ]---+
                  |               |
     (GPIO 0)  D0 | [ ]       [ ] | 5V   (USB 5V In / VBUS)
     (GPIO 1)  D1 | [ ]       [ ] | GND  (Ground)
     (GPIO 2)  D2 | [ ]       [ ] | 3V3  (3.3V Regulated Output)
    (GPIO 21)  D3 | [ ]       [ ] | D10  (GPIO 18 / SPI MOSI)
(SDA / GPIO 22) D4 | [ ]       [ ] | D9   (GPIO 20 / SPI MISO)
(SCL / GPIO 23) D5 | [ ]       [ ] | D8   (GPIO 19 / SPI SCK)
 (TX / GPIO 16) D6 | [ ]       [ ] | D7   (GPIO 17 / UART0 RX)
                  |    [R] [B]    |
                  +---------------+
                     Reset Boot
```

* **Processor**: 32-bit RISC-V High-Performance core @ up to 160 MHz + Low-Power core @ 20 MHz
* **Memory**: 512 KB SRAM, 4 MB SPI Flash
* **Wireless**: Wi-Fi 6 (802.11ax), Bluetooth 5.3 Low Energy, Zigbee 3.0 / Thread (802.15.4)
* **Onboard LED**: Yellow User LED on **GPIO 15** (Active-LOW)
* **USB Interface**: Native USB Type-C with integrated USB-Serial-JTAG controller
* **Buttons**: Reset (`R`) and Bootloader (`B`)

---

## 📚 Progressive Learning Curriculum

This workspace includes 8 structured, standalone learning projects in `labs/`:

| Lab | Name | Key Concepts | Hardware Involved |
| :---: | :--- | :--- | :--- |
| **01** | [**01_blinky**](file:///c:/Users/Wyatt/Documents/Zepher/SeedStudio_c6_test/labs/01_blinky/README.md) | Devicetree aliases, `gpio_dt_spec`, active-low polarity, kernel sleeps | Onboard Yellow LED (GPIO 15) |
| **02** | [**02_button_interrupt**](file:///c:/Users/Wyatt/Documents/Zepher/SeedStudio_c6_test/labs/02_button_interrupt/README.md) | Pull-up inputs, hardware edge interrupts, ISR callbacks, debouncing | Pin D1 (GPIO 1) & GND |
| **03** | [**03_usb_logging_and_shell**](file:///c:/Users/Wyatt/Documents/Zepher/SeedStudio_c6_test/labs/03_usb_logging_and_shell/README.md) | Zephyr interactive CLI shell, logging levels, custom shell commands | Native USB-Serial CDC |
| **04** | [**04_rtos_multithreading**](file:///c:/Users/Wyatt/Documents/Zepher/SeedStudio_c6_test/labs/04_rtos_multithreading/README.md) | Static threads (`K_THREAD_DEFINE`), priorities, message queues, semaphores | Dual-thread concurrency |
| **05** | [**05_timers_and_workqueues**](file:///c:/Users/Wyatt/Documents/Zepher/SeedStudio_c6_test/labs/05_timers_and_workqueues/README.md) | Software timers (`k_timer`), system workqueue deferral (bottom-half processing) | Kernel worker thread |
| **06** | [**06_pwm_fade**](file:///c:/Users/Wyatt/Documents/Zepher/SeedStudio_c6_test/labs/06_pwm_fade/README.md) | Devicetree overlays (`app.overlay`), ESP32-C6 LEDC PWM, breathing LED | Hardware PWM on GPIO 15 |
| **07** | [**07_i2c_scanner**](file:///c:/Users/Wyatt/Documents/Zepher/SeedStudio_c6_test/labs/07_i2c_scanner/README.md) | I2C driver API, bus scanning (0x01–0x7F), Linux-style `i2cdetect` matrix | D4 (SDA) & D5 (SCL) |
| **08** | [**08_ble_beacon**](file:///c:/Users/Wyatt/Documents/Zepher/SeedStudio_c6_test/labs/08_ble_beacon/README.md) | Bluetooth 5.3 LE stack, GAP non-connectable advertising, telemetry packets | 2.4 GHz radio (nRF Connect phone app) |

---

## ⚡ Quick Start Guide

### Step 1: Initialize Workspace & Toolchain
Open PowerShell in this directory and run the automated bootstrap:
```powershell
.\scripts\setup_workspace.ps1
```
*(This verifies your Python virtual environment, installs build dependencies, pulls the lightweight Zephyr modules, and installs the minimal Zephyr RISC-V SDK).*

### Step 2: Activate Environment
In any PowerShell terminal, run:
```powershell
. .\scripts\env.ps1
```
*(Sets up `PATH`, `ZEPHYR_BASE`, `ZEPHYR_SDK_INSTALL_DIR`, and exports board variables).*

### Step 3: Build & Flash Lab 1 (Blinky)
```powershell
# Build
west build -b xiao_esp32c6 labs\01_blinky -p auto

# Flash to board
west flash
# or: .\scripts\flash.ps1

# Monitor serial logs (115200 baud)
.\scripts\monitor.ps1
```

---

## 💻 VS Code Integration

Pre-configured tasks are available under `.vscode/tasks.json`:
* **`Ctrl + Shift + B`**: Build Active Lab
* **`Terminal` -> `Run Task`**:
  * `West: Pristine Rebuild Active Lab`
  * `West: Build Selected Lab...` (Interactive picker for Labs 1–8)
  * `West: Flash Firmware`
  * `West: Serial Monitor (115200 baud)`
  * `West: Build, Flash & Monitor (All-In-One)`
  * `West: Open Menuconfig`
  * `ESP: Check Connection & Chip Info`
  * `ESP: Erase Full Flash`

---

## 📂 Workspace Structure

```text
SeedStudio_c6_test/
├── .venv/                         # Local Python virtual environment
├── .vscode/                       # VS Code tasks, settings, launch & extensions
│   ├── tasks.json                 # Pre-configured build, flash, and monitor tasks
│   ├── settings.json              # PowerShell terminal profile & IntelliSense paths
│   └── launch.json                # JTAG debugging via built-in USB-JTAG OpenOCD
├── boards/                        # Out-of-tree board definition package
│   └── seeed/
│       └── xiao_esp32c6/          # Official DTS, pinctrl, bindings, and openocd.cfg
├── docs/                          # Deep reference guides
│   ├── XIAO_ESP32C6_HARDWARE.md   # Pinout, schematics, battery circuit, bootloader
│   ├── ZEPHYR_ARCHITECTURE_GUIDE.md# Mental model: Kconfig vs DTS vs Drivers
│   ├── ZEPHYR_CHEATSHEET.md       # West CLI & Kernel C macro quick reference
│   └── TROUBLESHOOTING.md         # COM ports, permission errors, flashing fixes
├── labs/                          # Progressive hands-on projects (01 to 08)
│   ├── 01_blinky/
│   ├── 02_button_interrupt/
│   ├── 03_usb_logging_and_shell/
│   ├── 04_rtos_multithreading/
│   ├── 05_timers_and_workqueues/
│   ├── 06_pwm_fade/
│   ├── 07_i2c_scanner/
│   └── 08_ble_beacon/
├── scripts/                       # PowerShell automation tools
│   ├── env.ps1                    # Environment loader with UTF-8 & SDK exports
│   ├── setup_workspace.ps1        # Full automated bootstrap
│   ├── install_sdk.ps1            # Lightweight Zephyr RISC-V SDK installer
│   ├── build.ps1                  # Convenient builder script
│   ├── flash.ps1                  # Auto-detecting COM port flasher
│   └── monitor.ps1                # 115200 baud serial monitor
└── west.yml                       # West manifest (filters hal_espressif, mbedtls)
```

---

## 📖 Additional Documentation
* [XIAO ESP32C6 Hardware Reference](file:///c:/Users/Wyatt/Documents/Zepher/SeedStudio_c6_test/docs/XIAO_ESP32C6_HARDWARE.md)
* [Zephyr Architecture & Mental Model](file:///c:/Users/Wyatt/Documents/Zepher/SeedStudio_c6_test/docs/ZEPHYR_ARCHITECTURE_GUIDE.md)
* [Zephyr API & West Cheatsheet](file:///c:/Users/Wyatt/Documents/Zepher/SeedStudio_c6_test/docs/ZEPHYR_CHEATSHEET.md)
* [Troubleshooting & FAQ](file:///c:/Users/Wyatt/Documents/Zepher/SeedStudio_c6_test/docs/TROUBLESHOOTING.md)
