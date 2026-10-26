# scripts/flash.ps1 - Flash firmware to Seeed Studio XIAO ESP32C6
[CmdletBinding()]
param(
    [string]$Port = "",
    [int]$Baud = 460800,
    [switch]$EraseFirst
)

$WorkspaceRoot = (Resolve-Path "$PSScriptRoot\..").Path
. "$PSScriptRoot\env.ps1"

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "     Flashing Firmware to Seeed Studio XIAO ESP32C6         " -ForegroundColor Yellow
Write-Host "============================================================" -ForegroundColor Cyan

# 1. Detect Available COM Ports
$ports = [System.IO.Ports.SerialPort]::GetPortNames()
if ($ports.Count -eq 0) {
    Write-Warning "No COM ports detected! Please ensure your XIAO ESP32C6 is plugged in via USB-C."
    Write-Host "Tip: If the board is not responding, enter Bootloader Mode manually:" -ForegroundColor Yellow
    Write-Host "     1. Press and HOLD the 'B' (Boot) button."
    Write-Host "     2. Press and RELEASE the 'R' (Reset) button."
    Write-Host "     3. RELEASE the 'B' (Boot) button."
    exit 1
}

if (-not $Port) {
    if ($ports.Count -eq 1) {
        $Port = $ports[0]
        Write-Host "Auto-detected single COM port: $Port" -ForegroundColor Green
    } else {
        Write-Host "Multiple COM ports detected: $($ports -join ', ')" -ForegroundColor Yellow
        $Port = $ports[0]
        Write-Host "Defaulting to: $Port (specify -Port COMx to override)" -ForegroundColor Gray
    }
}

if ($EraseFirst) {
    Write-Host ">>> Erasing chip flash..." -ForegroundColor Yellow
    python -m esptool --chip esp32c6 --port $Port erase_flash
}

Write-Host ">>> Attempting 'west flash' on port $Port at $Baud baud..." -ForegroundColor Cyan

# West flash runner
try {
    west flash --esp-port $Port --esp-baud-rate $Baud --esp-reset-type watchdog_reset
    if ($LASTEXITCODE -eq 0) {
        Write-Host "`n>>> Flashing Complete! Resetting board..." -ForegroundColor Green
        Write-Host ">>> To view serial logs: .\scripts\monitor.ps1 -Port $Port" -ForegroundColor Yellow
        exit 0
    }
} catch {
    Write-Warning "west flash encountered an error, trying direct esptool runner..."
}

# Fallback direct esptool invocation if build/zephyr exists
$buildBin = Join-Path $WorkspaceRoot "build\zephyr\zephyr.bin"
if (Test-Path $buildBin) {
    Write-Host ">>> Flashing zephyr.bin directly with esptool..." -ForegroundColor Cyan
    python -m esptool --chip esp32c6 --port $Port --baud $Baud --after watchdog_reset write_flash 0x0 $buildBin
    if ($LASTEXITCODE -eq 0) {
        Write-Host "`n>>> Flashing Succeeded via esptool!" -ForegroundColor Green
        Write-Host ">>> To view serial logs: .\scripts\monitor.ps1 -Port $Port" -ForegroundColor Yellow
    }
} else {
    Write-Error "No compiled binary found at $buildBin. Please run .\scripts\build.ps1 first."
}
