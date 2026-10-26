# scripts/setup_workspace.ps1 - Automated Setup for Seeed Studio XIAO ESP32C6 Zephyr Workspace
[CmdletBinding()]
param(
    [switch]$SkipSdkInstall,
    [switch]$SkipWestUpdate
)

$WorkspaceRoot = (Resolve-Path "$PSScriptRoot\..").Path
$env:PYTHONIOENCODING = "utf-8"

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "     Bootstrapping Zephyr Workspace for Seeed XIAO ESP32C6  " -ForegroundColor Yellow
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host " Workspace: $WorkspaceRoot"

# Step 1: Check Python
Write-Host "`n[1/4] Checking Python environment..." -ForegroundColor Green
$pyVer = py -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}.{sys.version_info.micro}')" 2>$null
if (-not $pyVer) {
    Write-Error "Python 3 is not found. Please install Python 3.10+ from python.org or via winget."
    exit 1
}
Write-Host "  Found Python $pyVer" -ForegroundColor White

# Step 2: Virtual Environment & Pip Packages
Write-Host "`n[2/4] Setting up Python virtual environment..." -ForegroundColor Green
$VenvDir = Join-Path $WorkspaceRoot ".venv"
if (-not (Test-Path $VenvDir)) {
    Write-Host "  Creating virtual environment at .venv..."
    py -m venv $VenvDir
}

$PipExe = Join-Path $VenvDir "Scripts\pip.exe"
$WestExe = Join-Path $VenvDir "Scripts\west.exe"

Write-Host "  Installing core build tools (west, cmake, ninja, esptool, pyelftools)..."
& $PipExe install --upgrade pip west cmake ninja esptool pyelftools cbor2 packaging tabulate psutil windows-curses

# Step 3: West Update
Write-Host "`n[3/4] Updating Zephyr source trees via West..." -ForegroundColor Green
if ($SkipWestUpdate) {
    Write-Host "  Skipping west update as requested." -ForegroundColor Yellow
} else {
    $env:PATH = "$VenvDir\Scripts;$env:PATH"
    $env:ZEPHYR_BASE = Join-Path $WorkspaceRoot "zephyr"
    
    Write-Host "  Running 'west update' (fetching Zephyr and hal_espressif)..."
    & $WestExe update
    
    # Install Zephyr's internal Python requirements
    $ZephyrReqs = Join-Path $WorkspaceRoot "zephyr\scripts\requirements.txt"
    if (Test-Path $ZephyrReqs) {
        Write-Host "  Installing Zephyr scripts requirements.txt..."
        & $PipExe install -r $ZephyrReqs
    }
}

# Step 4: Zephyr SDK Check
Write-Host "`n[4/4] Checking Zephyr SDK (RISC-V toolchain)..." -ForegroundColor Green
$SdkFound = $false
$SdkCandidates = @(
    (Join-Path $WorkspaceRoot "tools\zephyr-sdk"),
    (Join-Path $HOME "zephyr-sdk-0.17.0"),
    (Join-Path $HOME "zephyr-sdk"),
    "C:\zephyr-sdk-0.17.0",
    "C:\zephyr-sdk"
)

foreach ($cand in $SdkCandidates) {
    if (Test-Path $cand) {
        Write-Host "  Found Zephyr SDK at: $cand" -ForegroundColor White
        $SdkFound = $true
        $env:ZEPHYR_SDK_INSTALL_DIR = $cand
        break
    }
}

if (-not $SdkFound -and -not $SkipSdkInstall) {
    Write-Host "  Zephyr SDK not detected. Installing Zephyr SDK v0.17.0 (RISC-V toolchain)..." -ForegroundColor Yellow
    & "$PSScriptRoot\install_sdk.ps1"
}

Write-Host "`n============================================================" -ForegroundColor Cyan
Write-Host "  Workspace setup complete! " -ForegroundColor Green
Write-Host "  To activate your environment in any PowerShell terminal:" -ForegroundColor White
Write-Host "     . .\scripts\env.ps1" -ForegroundColor Yellow
Write-Host "============================================================" -ForegroundColor Cyan
