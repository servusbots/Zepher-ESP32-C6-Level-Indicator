# Bulls-Eye Level Indicator with MPU-6050 & SSD1306 OLED

A real-time, precision digital level indicator application for the **Seeed Studio XIAO ESP32C6** (RISC-V) developed with **Zephyr RTOS**.

---

## 🎯 Features

1. **5-Second Automatic Zero-Offset Calibration**:
   - On power-up/boot, the system samples the MPU-6050 accelerometer for 5.0 seconds.
   - Shows an animated progress bar and countdown on the OLED display (`Ready in: X.Xs`).
   - Computes reference offsets ($\text{bias}_X, \text{bias}_Y$) for whatever surface the device starts on.
2. **Bulls-Eye Circular Level Target**:
   - Centered target reticle at $(64, 32)$ on the 128x64 display.
   - 3 concentric reference rings:
     - **Center Bullseye Ring** ($R = 5\text{ px}$): Target level zone ($< 1.0^\circ$).
     - **Middle Reference Ring** ($R = 14\text{ px}$): Moderate tilt zone ($\sim 5^\circ$).
     - **Outer Reference Ring** ($R = 24\text{ px}$): High tilt zone ($\sim 10^\circ$).
   - Precision crosshairs with central bullseye clearance.
3. **Dynamic Bubble Indicator**:
   - Smoothly tracked filled circular bubble ($R = 3\text{ px}$) driven by Pitch & Roll angles with exponential moving average filtering ($\alpha = 0.30$) for stable, jitter-free response.
   - Digital angle telemetry readouts: `R: +0.0°` (Roll) and `P: -0.0°` (Pitch).
4. **Active Screen Flash & Strobe on Level**:
   - When the bubble settles inside the center target ring ($|\text{roll}| < 1.0^\circ$ and $|\text{pitch}| < 1.0^\circ$), the screen enters **LEVEL mode**:
     - The entire OLED display strobes/flashes at $3.3\text{ Hz}$ ($150\text{ ms}$ invert duty cycle).
     - Status label `*LEVEL*` appears.
     - The onboard yellow LED (`GPIO 15`) pulses synchronously.
5. **USB Telemetry**:
   - Streams live roll/pitch angles and level status over the native USB-Serial-JTAG interface (`COM18`).

---

## 🔌 Hardware Wiring

Both the **MPU-6050** and the **0.96" SSD1306 OLED** share the single hardware `I2C_0` bus on the XIAO ESP32C6:

| Device Pin | XIAO ESP32C6 Pin | Function | Notes |
|:---|:---|:---|:---|
| **VCC** | **3V3** | Power (3.3V) | Connect both OLED and MPU6050 VCC to 3.3V |
| **GND** | **GND** | Ground | Common ground |
| **SDA** | **D4 (GPIO 22)** | I2C Data | Shared bus (Internal pull-ups enabled) |
| **SCL** | **D5 (GPIO 23)** | I2C Clock | Shared bus (400 kHz Fast-Mode) |

### I2C Bus Addresses:
- **SSD1306 OLED**: `0x3C`
- **MPU-6050 IMU**: `0x68` (AD0 pin grounded / default)

---

## 🚀 Building & Flashing

### 1. Build the Application:
```powershell
. .\scripts\env.ps1
west build -p always -b xiao_esp32c6/esp32c6/hpcore --sysbuild .\levelindicator
```

### 2. Flash to XIAO ESP32C6:
```powershell
.\scripts\flash.ps1 -Port COM18
```

### 3. Open Serial Telemetry:
```powershell
.\scripts\monitor.ps1 -Port COM18
```

---

## 📐 Sensor Mathematics

Pitch ($\theta$) and Roll ($\phi$) are derived from the 3-axis accelerometer vector $\mathbf{a} = [a_x, a_y, a_z]^T$:

$$\text{Roll} = \text{atan2}(a_y', a_z) \times \frac{180.0}{\pi}$$
$$\text{Pitch} = \text{atan2}(-a_x', \sqrt{a_y'^2 + a_z^2}) \times \frac{180.0}{\pi}$$

Where:
$$a_x' = a_x - \text{bias}_x, \quad a_y' = a_y - \text{bias}_y$$
