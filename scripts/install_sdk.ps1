# scripts/install_sdk.ps1 - Automated Zephyr SDK (RISC-V) Installer for Seeed XIAO ESP32C6
param(
    [string]$InstallDir = (Join-Path $HOME "zephyr-sdk-0.17.0"),
    [string]$Version = "0.17.0"
)

$ErrorActionPreference = "Stop"
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "     Zephyr SDK $Version Installer (RISC-V Toolchain)       " -ForegroundColor Yellow
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host " Target Directory: $InstallDir"

if (-not (Test-Path $InstallDir)) {
    New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
}

$TempDir = Join-Path $PSScriptRoot "..\.cache"
if (-not (Test-Path $TempDir)) {
    New-Item -ItemType Directory -Force -Path $TempDir | Out-Null
}

$BaseUrl = "https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v$Version"
$MinimalArchive = "zephyr-sdk-${Version}_windows-x86_64_minimal.7z"
$RiscvArchive = "toolchain_windows-x86_64_riscv64-zephyr-elf.7z"

$MinimalPath = Join-Path $TempDir $MinimalArchive
$RiscvPath = Join-Path $TempDir $RiscvArchive

# Step 1: Download Minimal SDK
if (-not (Test-Path $MinimalPath)) {
    Write-Host "[1/4] Downloading $MinimalArchive..." -ForegroundColor Green
    $Uri = "$BaseUrl/$MinimalArchive"
    Invoke-WebRequest -Uri $Uri -OutFile $MinimalPath
} else {
    Write-Host "[1/4] Minimal bundle archive already downloaded." -ForegroundColor Gray
}

# Step 2: Download RISC-V Toolchain (ESP32-C6 target)
if (-not (Test-Path $RiscvPath)) {
    Write-Host "[2/4] Downloading $RiscvArchive (~100 MB)..." -ForegroundColor Green
    $Uri = "$BaseUrl/$RiscvArchive"
    Invoke-WebRequest -Uri $Uri -OutFile $RiscvPath
} else {
    Write-Host "[2/4] RISC-V toolchain archive already downloaded." -ForegroundColor Gray
}

# Step 3: Extract archives
Write-Host "[3/4] Extracting SDK files to $InstallDir..." -ForegroundColor Green
Write-Host "  -> Extracting minimal host tools..."
tar.exe -xf $MinimalPath -C $InstallDir
Write-Host "  -> Extracting RISC-V 64 toolchain (this may take 15-30 seconds)..."
tar.exe -xf $RiscvPath -C $InstallDir

# Step 4: Run setup.cmd to register CMake packages
$SetupCmd = Join-Path $InstallDir "setup.cmd"
if (Test-Path $SetupCmd) {
    Write-Host "[4/4] Registering Zephyr SDK with CMake..." -ForegroundColor Green
    & cmd.exe /c "$SetupCmd -c"
}

# Set Environment Variable
$env:ZEPHYR_SDK_INSTALL_DIR = $InstallDir
[Environment]::SetEnvironmentVariable("ZEPHYR_SDK_INSTALL_DIR", $InstallDir, "User")

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host " Zephyr SDK $Version (RISC-V) installed successfully!" -ForegroundColor Green
Write-Host " ZEPHYR_SDK_INSTALL_DIR set to: $InstallDir" -ForegroundColor White
Write-Host "============================================================" -ForegroundColor Cyan
