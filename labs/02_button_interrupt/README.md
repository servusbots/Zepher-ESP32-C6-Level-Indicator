# Lab 02: Button Inputs & Hardware Interrupts

In embedded development, polling a pin in a loop wastes battery power and CPU cycles. Zephyr provides an event-driven interrupt architecture that allows the CPU to sleep until hardware events occur.

---

## 🎯 Objectives
1. Configure an input GPIO with internal pull-up (`GPIO_PULL_UP`).
2. Set up hardware edge interrupts (`gpio_pin_interrupt_configure`).
3. Register an Interrupt Service Routine (ISR) callback.
4. Implement software debouncing using `k_uptime_get_32()`.

---

## 🔌 Hardware Setup
* **Board**: Seeed Studio XIAO ESP32C6
* **Button Pin**: **D1 (GPIO 1)**
* **Ground Pin**: **GND** (pin next to 3V3 / D0)

You can use a standard momentary tactile button, or simply touch a jumper wire between **D1** and **GND**!

```text
       XIAO ESP32C6
     +--------------+
 D0  | [ ]      [ ] | 5V
 D1  | [x]--\   [ ] | GND
 D2  | [ ]   \  [ ] | 3V3
 D3  | [ ]    o [x] | GND  <-- Connect switch or wire
     |     [BUTTON] |
```

---

## 🚀 Building & Flashing

```powershell
. .\scripts\env.ps1
west build -b xiao_esp32c6 labs\02_button_interrupt -p auto
west flash
.\scripts\monitor.ps1
```

---

## 🧠 Key Zephyr Concepts Learned
* `gpio_pin_interrupt_configure()`: Enables interrupt generation on edge transitions (rising, falling, or both).
* `gpio_init_callback()` / `gpio_add_callback()`: Registers the function to run when the hardware IRQ fires.
* **ISR Context Rules**: Interrupt callbacks run in interrupt context—never call blocking APIs like `k_sleep()` or `k_msleep()` inside an ISR!
