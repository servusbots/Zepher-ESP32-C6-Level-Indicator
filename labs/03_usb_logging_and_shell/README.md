# Lab 03: USB-Serial Logging & Interactive Shell

Zephyr includes an enterprise-grade command-line shell that turns your embedded board into an interactive command prompt. This is invaluable for debugging, diagnostics, sensor calibration, and hardware verification without re-flashing firmware!

---

## 🎯 Objectives
1. Learn how to configure the **Zephyr Shell** over USB CDC / USB-Serial-JTAG.
2. Understand the Zephyr **Logging subsystem** (`LOG_MODULE_REGISTER`, `LOG_INF`, `LOG_DBG`, etc.).
3. Write and register custom shell commands with arguments and subcommands (`SHELL_CMD_REGISTER`, `SHELL_STATIC_SUBCMD_SET_CREATE`).

---

## 🚀 Building & Flashing

```powershell
. .\scripts\env.ps1
west build -b xiao_esp32c6 labs\03_usb_logging_and_shell -p auto
west flash
.\scripts\monitor.ps1
```

---

## 🖥️ Interactive Shell Commands
Once the serial monitor connects, hit **Enter** to display the shell prompt (`xiao-c6:~$ `):

```text
xiao-c6:~$ help
Please press the <Tab> button to see all available commands.
  board_info   : Show board metadata and system uptime
  led          : Control onboard LED (on, off, toggle)
  kernel       : Kernel commands
  log          : Logger commands

xiao-c6:~$ board_info
========================================
 Board:      Seeed Studio XIAO ESP32C6
 SoC:        Espressif ESP32-C6 (RISC-V)
 Core Clock: 160 MHz
 OS Version: Zephyr 3.7.0
 Uptime:     12543 ms
========================================

xiao-c6:~$ led on
LED turned ON

xiao-c6:~$ led off
LED turned OFF

xiao-c6:~$ led toggle
LED toggled
```

You can even adjust runtime log levels on the fly:
```text
xiao-c6:~$ log status
xiao-c6:~$ log enable dbg lab03_shell
```
