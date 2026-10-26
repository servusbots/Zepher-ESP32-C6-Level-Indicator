# Lab 01: Blinky & Devicetree Fundamentals

Welcome to your first Zephyr RTOS project on the **Seeed Studio XIAO ESP32C6**!

---

## 🎯 Objectives
1. Learn how Zephyr separates **hardware descriptions** (Devicetree) from **application logic** (C source).
2. Understand the `struct gpio_dt_spec` data structure.
3. Learn how Zephyr handles **Active-Low** vs **Active-High** polarity automatically.
4. Build, flash, and view runtime serial logs.

---

## 🔌 Hardware Breakdown
On the Seeed Studio XIAO ESP32C6:
* **Target Board**: `xiao_esp32c6`
* **Onboard LED**: Yellow LED located near the USB-C port.
* **Microcontroller Pin**: **GPIO 15**
* **Polarity**: **Active LOW** (Setting the GPIO low sinks current and lights the LED).

In the board's Devicetree (`boards/seeed/xiao_esp32c6/xiao_esp32c6_hpcore.dts`):
```dts
leds: leds {
    compatible = "gpio-leds";
    yellow_led: led_0 {
        gpios = <&gpio0 15 GPIO_ACTIVE_LOW>;
        label = "User LED1";
    };
};

aliases {
    led0 = &yellow_led;
};
```

Because of `GPIO_ACTIVE_LOW`, calling `gpio_pin_set_dt(&led, 1)` means **turn the LED ON**, regardless of whether the physical circuit requires 3.3V or 0V. Zephyr's driver handles the inversion under the hood!

---

## 🚀 Building & Flashing

### From PowerShell Terminal:
```powershell
# 1. Activate your workspace environment
. .\scripts\env.ps1

# 2. Build for XIAO ESP32C6
west build -b xiao_esp32c6 labs\01_blinky -p auto

# 3. Flash to the board
west flash
# or: .\scripts\flash.ps1

# 4. View serial output
.\scripts\monitor.ps1
```

### From VS Code:
1. Press `Ctrl + Shift + B` (or run task: `West: Build Active Lab`).
2. Run task `West: Flash Firmware`.
3. Run task `West: Serial Monitor (115200 baud)`.

---

## 🧠 Key Zephyr Concepts Learned
* `DT_ALIAS(led0)`: Resolves hardware node alias defined in `.dts`.
* `GPIO_DT_SPEC_GET()`: Statically extracts driver pointer, pin number, and polarity flags.
* `gpio_is_ready_dt()`: Checks that the hardware subsystem is initialized before calling driver methods.
* `k_msleep(ms)`: Relinquishes CPU control to other threads (or low-power idle) for the given duration.
