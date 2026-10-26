# scripts/env.ps1 - Environment loader for Zephyr RTOS on Seeed Studio XIAO ESP32C6
# Usage: . .\scripts\env.ps1

$WorkspaceRoot = (Resolve-Path "$PSScriptRoot\..").Path

# 1. Force UTF-8 Encoding in PowerShell to prevent CP1252 truncation/errors
$env:PYTHONIOENCODING = "utf-8"
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

# 2. Activate Virtual Environment
$VenvScripts = Join-Path $WorkspaceRoot ".venv\Scripts"
if (Test-Path $VenvScripts) {
    if ($env:PATH -notlike "*$VenvScripts*") {
        $env:PATH = "$VenvScripts;$env:PATH"
    }
} else {
    Write-Warning "Python virtual environment not found at '$VenvScripts'. Run .\scripts\setup_workspace.ps1 to create it."
}

# 3. Configure Zephyr Base Directory
$ZephyrBase = Join-Path $WorkspaceRoot "zephyr"
if (Test-Path $ZephyrBase) {
    $env:ZEPHYR_BASE = $ZephyrBase
} else {
    # Default to expected relative path
    $env:ZEPHYR_BASE = $ZephyrBase
}

# 4. Configure Board Target
$env:BOARD = "xiao_esp32c6"

# 5. Detect and Configure Zephyr SDK
$SdkCandidates = @(
    (Join-Path $WorkspaceRoot "tools\zephyr-sdk"),
    (Join-Path $HOME "zephyr-sdk-1.0.1"),
    (Join-Path $HOME "zephyr-sdk"),
    (Join-Path $HOME "zephyr-sdk-0.17.0"),
    "C:\zephyr-sdk-1.0.1",
    "C:\zephyr-sdk-0.17.0",
    "C:\zephyr-sdk"
)

$FoundSdk = $null
foreach ($cand in $SdkCandidates) {
    if (Test-Path $cand) {
        $FoundSdk = $cand
        break
    }
}

if ($FoundSdk) {
    $env:ZEPHYR_SDK_INSTALL_DIR = $FoundSdk
} else {
    $env:ZEPHYR_SDK_INSTALL_DIR = (Join-Path $HOME "zephyr-sdk-1.0.1")
}

# 6. Display Welcome Banner
Write-Host ""
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "  Zephyr RTOS Development Environment - Seeed XIAO ESP32C6  " -ForegroundColor Yellow
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host " Workspace Root: $WorkspaceRoot"
Write-Host " Python:        $(py -c 'import sys; print(sys.executable)' 2>$null)"
Write-Host " Board Target:  $env:BOARD" -ForegroundColor Green
Write-Host " Zephyr Base:   $env:ZEPHYR_BASE"
Write-Host " Zephyr SDK:    $env:ZEPHYR_SDK_INSTALL_DIR" -ForegroundColor $(if (Test-Path $env:ZEPHYR_SDK_INSTALL_DIR) { "Green" } else { "DarkYellow" })
if (-not (Test-Path $env:ZEPHYR_SDK_INSTALL_DIR)) {
    Write-Host "                (SDK not detected at this path. Run .\scripts\install_sdk.ps1 to install)" -ForegroundColor DarkYellow
}
Write-Host "------------------------------------------------------------" -ForegroundColor Cyan
Write-Host " Quick Commands:" -ForegroundColor White
Write-Host "   Build Lab 1:     west build -b xiao_esp32c6 labs\01_blinky -p auto" -ForegroundColor Gray
Write-Host "   Flash to Board:  west flash" -ForegroundColor Gray
Write-Host "   Serial Monitor:  .\scripts\monitor.ps1" -ForegroundColor Gray
Write-Host "   Helper Script:   .\scripts\build.ps1 labs\01_blinky" -ForegroundColor Gray
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host ""
