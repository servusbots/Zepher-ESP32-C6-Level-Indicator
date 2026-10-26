# Troubleshooting & FAQ Guide

Common issues encountered when developing with Zephyr RTOS on the **Seeed Studio XIAO ESP32C6** under Windows, along with step-by-step solutions.

---

## 🔌 1. Serial Port / Flashing Issues

### Symptom: `No COM ports detected` or `Failed to connect to ESP32-C6: No serial data received`
**Cause**: The chip may be in deep sleep, running a crashed program, or the USB port requires a state reset.
**Solution**:
1. Put the board into **ROM Download Mode**:
   - Press and **HOLD** the **`B`** (Boot) button.
   - While holding `B`, tap and release the **`R`** (Reset) button.
   - Release the **`B`** button.
2. In Windows Device Manager, verify that `USB JTAG/serial debug unit` or `Silicon Labs CP210x` / `USB-Enhanced-SERIAL` appears under **Ports (COM & LPT)**.
3. Retry flashing:
   ```powershell
   .\scripts\flash.ps1
   ```

### Symptom: `Access is denied` or `PermissionError: [Errno 13] Could not open port 'COMx'`
**Cause**: Another application is currently locking the COM port (e.g. Serial Monitor, Arduino IDE, PuTTY, or another terminal session).
**Solution**:
* Close any active miniterm or serial monitor windows before running `west flash`.
* In VS Code, kill any running Serial Monitor terminal tabs.

---

## 💻 2. PowerShell & Environment Issues

### Symptom: `File env.ps1 cannot be loaded because running scripts is disabled on this system`
**Cause**: Windows PowerShell default security policy blocks script execution.
**Solution**:
Run the following command in PowerShell:
```powershell
Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy RemoteSigned -Force
```

### Symptom: `UnicodeEncodeError: 'charmap' codec can't encode characters...`
**Cause**: Python is attempting to output UTF-8 progress characters in a Windows terminal using legacy `cp1252` encoding.
**Solution**:
Set the environment variable:
```powershell
$env:PYTHONIOENCODING = "utf-8"
```
*(Note: `.\scripts\env.ps1` sets this automatically).*

---

## ⚙️ 3. Zephyr SDK & Toolchain Issues

### Symptom: `Zephyr SDK not found` or `CMake Error at cmake/toolchain/zephyr/generic.cmake: No toolchain found`
**Cause**: `ZEPHYR_SDK_INSTALL_DIR` is not set or points to an invalid directory.
**Solution**:
1. Run the workspace automated SDK installer:
   ```powershell
   .\scripts\install_sdk.ps1
   ```
2. Verify the path exists and contains `riscv64-zephyr-elf`:
   ```powershell
   Test-Path "$env:ZEPHYR_SDK_INSTALL_DIR\riscv64-zephyr-elf"
   ```

---

## 🔨 4. Build System & Devicetree Issues

### Symptom: `devicetree error: alias 'led0' refers to unknown node`
**Cause**: The board definition or overlay is missing the expected alias.
**Solution**:
Ensure you are targeting `xiao_esp32c6` with `-DBOARD_ROOT`:
```powershell
west build -b xiao_esp32c6 labs\01_blinky -p always -- -DBOARD_ROOT="C:\Users\Wyatt\Documents\Zepher\SeedStudio_c6_test"
```

### Symptom: `Kconfig warnings` or stale cached settings
**Cause**: Build artifacts from a previous configuration are lingering.
**Solution**:
Perform a pristine rebuild:
```powershell
west build -p always
```
or delete the `build/` directory completely.
