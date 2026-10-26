# Lab 06: Hardware PWM & Breathing LED

Pulse Width Modulation (PWM) allows microcontrollers to generate variable average power using rapid digital pulsing. It is essential for LED brightness fading, servo motors, DC motor speed control, and audio tones.

---

## 🎯 Objectives
1. Understand how **Devicetree Overlays** (`app.overlay`) extend and customize default board pinouts.
2. Configure the ESP32-C6 **LEDC (LED Controller)** PWM peripheral.
3. Use Zephyr's standard `struct pwm_dt_spec` and `pwm_set_pulse_dt()` APIs.
4. Generate a smooth breathing light effect without blocking CPU execution.

---

## 🔌 Hardware Details
* **Pin**: GPIO 15 (Onboard Yellow LED)
* **Frequency**: 1000 Hz (Period = 1,000,000 ns)
* **Duty Cycle**: Dynamically varied from 0% to 100% and back down in 2% increments.

---

## 🚀 Building & Flashing

```powershell
. .\scripts\env.ps1
west build -b xiao_esp32c6 labs\06_pwm_fade -p auto
west flash
.\scripts\monitor.ps1
```
