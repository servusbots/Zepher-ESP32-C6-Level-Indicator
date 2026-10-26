# Seeed Studio XIAO ESP32C6 - Hardware Reference Guide

The **Seeed Studio XIAO ESP32C6** is an ultra-compact (21 x 17.5 mm) IoT development board powered by the **Espressif ESP32-C6 SoC**. It features a modern 32-bit RISC-V dual-core architecture and multi-protocol wireless connectivity.

---

## 📋 Technical Specifications

| Feature | Specification |
| :--- | :--- |
| **SoC** | Espressif ESP32-C6 |
| **CPU (High-Performance)** | 32-bit RISC-V single-core @ up to 160 MHz |
| **CPU (Low-Power Core)** | 32-bit RISC-V LP core @ up to 20 MHz |
| **SRAM** | 512 KB HP SRAM + 16 KB LP SRAM |
| **ROM** | 320 KB ROM |
| **Flash Memory** | 4 MB Onboard SPI Flash |
| **Wi-Fi** | 2.4 GHz Wi-Fi 6 (802.11ax/b/g/n), 20/40 MHz bandwidth, Target Wake Time (TWT) |
| **Bluetooth** | Bluetooth 5.3 Low Energy (LE), Long Range (Coded PHY), 2 Mbps PHY |
| **802.15.4 Radio** | Thread 1.3, Zigbee 3.0, Matter compliant |
| **Antenna** | Onboard ceramic antenna + external U.FL connector (selectable via 0-ohm resistor/RF switch) |
| **USB Interface** | Native USB Type-C with integrated USB-Serial-JTAG controller |
| **Buttons** | Reset Button ('R') + Boot Button ('B') |
| **Status Indicators** | Yellow User LED (GPIO 15, Active Low), Green Power/Charge LED |
| **Battery Management**| Built-in lithium battery charger (pads on underside) |
| **Dimensions** | 21 mm x 17.5 mm x 4.8 mm |

---

## 📌 Pinout & Peripheral Mapping

The XIAO 14-pin form factor exposes 11 digital I/O pins (D0–D10) on a 2.54mm pitch:

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

### Detailed Pin Multiplexing Table

| XIAO Pin | ESP32-C6 GPIO | Default Zephyr Function | ADC Channel | Hardware Peripherals |
| :---: | :---: | :--- | :---: | :--- |
| **D0** | `GPIO0` | General Purpose I/O | `ADC1_CH0` | PWM, RTC |
| **D1** | `GPIO1` | General Purpose I/O | `ADC1_CH1` | PWM, RTC |
| **D2** | `GPIO2` | General Purpose I/O | `ADC1_CH2` | PWM, RTC |
| **D3** | `GPIO21` | General Purpose I/O | `ADC1_CH3` | PWM |
| **D4** | `GPIO22` | **I2C0 SDA** | `ADC1_CH4` | PWM, I2C Data |
| **D5** | `GPIO23` | **I2C0 SCL** | `ADC1_CH5` | PWM, I2C Clock |
| **D6** | `GPIO16` | **UART0 TX** | `ADC1_CH6` | Hardware Serial TX |
| **D7** | `GPIO17` | **UART0 RX** | — | Hardware Serial RX |
| **D8** | `GPIO19` | **SPI2 SCK** | `ADC1_CH7` | SPI Clock |
| **D9** | `GPIO20` | **SPI2 MISO** | — | SPI Master-In-Slave-Out |
| **D10**| `GPIO18` | **SPI2 MOSI** | — | SPI Master-Out-Slave-In |
| **LED**| `GPIO15` | **User LED (Yellow)** | — | Active-LOW sink |

---

## 🔘 Hardware Buttons & Bootloader Mode

The board contains two surface-mount micro-buttons located near the bottom edge:
* **`R` (Reset)**: Resets the chip.
* **`B` (Boot)**: Used to enter the ROM Bootloader mode.

### How to enter ROM Download / Bootloader Mode:
If your board is unresponsive or locked by faulty firmware:
1. Press and **HOLD** the **`B`** (Boot) button.
2. While holding `B`, press and release the **`R`** (Reset) button.
3. Release the **`B`** button.
4. The board will now appear as an Espressif ROM serial device, ready to be flashed via `west flash` or `esptool.py`.

---

## 🔋 Battery Charging & Power Options
* **USB-C**: Supplies 5V to the onboard 3.3V low-dropout (LDO) regulator.
* **Battery Pads**: Located on the underside of the PCB. Connect a 3.7V single-cell Lithium-ion / LiPo battery.
* When powered via USB-C, the board automatically charges an attached battery at up to 100mA.
