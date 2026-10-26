# Zephyr RTOS Architecture & Concepts Guide

If you are coming from **Arduino** or **bare FreeRTOS**, Zephyr represents a major leap forward in professional firmware engineering. Zephyr is a modern, scalable, secure, and vendor-neutral real-time operating system maintained by the Linux Foundation.

---

## 🏛️ The Three Pillars of Zephyr

Every Zephyr application is built around three synchronized pillars:

```text
 +-------------------------------------------------------------------------+
 |                              APPLICATION                                |
 +--------------------+-------------------------------+--------------------+
                      |                               |
       [1] KCONFIG    v               [2] DEVICETREE  v          [3] C CODE
     +-----------------+             +-----------------+     +-------------+
     | Feature Toggles |             | Hardware Layout |     | Application |
     |  (prj.conf)     |             |  (.dts, .overlay)|     |   Logic     |
     | - Enable BLE    |             | - Pin numbers   |     | (main.c)    |
     | - Enable Shell  |             | - Clock speeds  |     |             |
     | - Stack sizes   |             | - Bus addresses |     |             |
     +--------+--------+             +--------+--------+     +------+------+
              |                               |                     |
              +--------------+----------------+---------------------+
                             |
                             v
                 [Zephyr Build System (CMake + Ninja)]
                             |
                             v
                 [Firmware Binary (zephyr.bin)]
```

---

### Pillar 1: Kconfig (`prj.conf`)
* **What it does**: Controls **features, protocols, memory limits, and subsystems**.
* **Analogy**: Like compiler `#define` flags, but structured with dependency checking.
* **Examples**:
  ```ini
  CONFIG_GPIO=y                # Enable GPIO driver subsystem
  CONFIG_BT=y                  # Enable Bluetooth LE stack
  CONFIG_LOG=y                 # Enable Logging subsystem
  CONFIG_MAIN_STACK_SIZE=2048  # Set main thread stack to 2 KB
  ```

---

### Pillar 2: Devicetree (`.dts` & `.overlay`)
* **What it does**: Describes **physical hardware topology** (pins, buses, addresses, clocks) completely decoupled from C code.
* **Why it matters**: You never hardcode `digitalWrite(15, HIGH)`. Instead, you reference logical devicetree aliases like `led0`. If you port your code to an STM32 or nRF52, your C code doesn't change—only the devicetree changes!
* **Hierarchy**:
  1. Base SoC DTS (`esp32c6.dtsi`)
  2. Board DTS (`xiao_esp32c6_hpcore.dts`)
  3. Application Overlay (`app.overlay`) - overrides board defaults for a specific project.

---

### Pillar 3: Zephyr Driver Model (`DEVICE_DT_GET`)
In Zephyr, hardware peripherals are accessed through standardized driver APIs:
* **`gpio_dt_spec`**: GPIO pin, port, and polarity flags.
* **`i2c_dt_spec`**: I2C controller and 7-bit slave address.
* **`spi_dt_spec`**: SPI controller, frequency, and chip-select line.
* **`pwm_dt_spec`**: PWM controller, channel, and period.

---

## ⚙️ Kernel Concurrency Primitives

Zephyr provides an extensive suite of real-time multi-threading tools:

### 1. Threads (`K_THREAD_DEFINE`)
* **Preemptive Priorities**: Positive numbers (0 to 15). Lower number = higher priority.
* **Cooperative Priorities**: Negative numbers (-15 to -1). Once running, a cooperative thread cannot be preempted except when it calls `k_yield()` or sleeps.

### 2. Message Queues (`k_msgq`)
* Fixed-size ring buffers for passing structured data between threads.
* Thread-safe, non-blocking or blocking with timeout.

### 3. Semaphores (`k_sem`)
* Binary or counting flags used to signal events between ISRs and threads.
* `k_sem_give()` can be called safely from interrupt context!

### 4. Workqueues (`k_work`)
* The standard mechanism to defer heavy processing from an ISR to a thread (Bottom-Half processing).
* Zephyr includes a dedicated `k_sys_work_q` system workqueue thread.

---

## 🔄 The Zephyr Build Pipeline

When you execute `west build -b xiao_esp32c6 labs\01_blinky`:
1. **CMake** reads `CMakeLists.txt` and discovers Zephyr package.
2. **Kconfig** reads default board configs + `prj.conf` and generates `build/zephyr/include/generated/autoconf.h`.
3. **Devicetree Compiler (dtc)** merges board DTS with any `app.overlay` and outputs `build/zephyr/include/generated/devicetree_generated.h`.
4. **Ninja** invokes the RISC-V cross-compiler (`riscv64-zephyr-elf-gcc`).
5. **Esptool** packages the compiled ELF into `zephyr.bin` ready to flash!
