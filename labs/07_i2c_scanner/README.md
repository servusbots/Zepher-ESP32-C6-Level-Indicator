# Lab 07: I2C Bus & Peripheral Scanner

The Inter-Integrated Circuit (I2C) bus is the universal communication protocol for embedded sensors, displays, and real-time clocks. 

Before writing a sensor driver, you need to verify wiring and address communication. This lab implements an `i2cdetect` diagnostic scanner in Zephyr.

---

## 🎯 Objectives
1. Understand the Zephyr I2C API (`i2c_transfer`, `struct i2c_msg`).
2. Scan 7-bit addresses (0x03 to 0x77) using read transactions.
3. Print a real-time hexadecimal address grid.

---

## 🔌 Hardware Wiring
Connect your I2C sensor or display to the XIAO ESP32C6 header:
* **VCC**: Connect to **3V3**
* **GND**: Connect to **GND**
* **SDA**: Connect to **D4** (GPIO 22)
* **SCL**: Connect to **D5** (GPIO 23)

---

## 🚀 Building & Flashing

```powershell
. .\scripts\env.ps1
west build -b xiao_esp32c6 labs\07_i2c_scanner -p auto
west flash
.\scripts\monitor.ps1
```

### Sample Output:
```text
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- 3c -- -- --   <-- 0x3C Discovered! (SSD1306 OLED)
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- 68 -- -- -- -- -- -- --   <-- 0x68 Discovered! (MPU6050 IMU)
70: -- -- -- -- -- -- -- -- 

>>> Discovered 2 I2C device(s) on bus I2C_0.
```
