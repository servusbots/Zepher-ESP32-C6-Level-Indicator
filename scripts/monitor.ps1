# scripts/monitor.ps1 - Open Serial Monitor for Seeed Studio XIAO ESP32C6
[CmdletBinding()]
param(
    [string]$Port = "",
    [int]$Baud = 115200
)

$WorkspaceRoot = (Resolve-Path "$PSScriptRoot\..").Path
. "$PSScriptRoot\env.ps1"

$ports = [System.IO.Ports.SerialPort]::GetPortNames()
if ($ports.Count -eq 0) {
    Write-Warning "No serial COM ports found! Connect your XIAO ESP32C6 board via USB-C."
    exit 1
}

if (-not $Port) {
    if ($ports.Count -eq 1) {
        $Port = $ports[0]
    } else {
        Write-Host "Available COM ports: $($ports -join ', ')" -ForegroundColor Yellow
        $Port = $ports[0]
        Write-Host "Defaulting to: $Port" -ForegroundColor Gray
    }
}

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host " Opening Serial Monitor on $Port @ $Baud baud..." -ForegroundColor Green
Write-Host " (Press Ctrl+] or Ctrl+C to exit miniterm)" -ForegroundColor DarkGray
Write-Host "============================================================" -ForegroundColor Cyan

python -m serial.tools.miniterm $Port $Baud --raw
