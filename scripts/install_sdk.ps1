# scripts/install_sdk.ps1 - Automated Zephyr SDK (RISC-V) Installer for Seeed XIAO ESP32C6
param(
    [string]$InstallDir = (Join-Path $HOME "zephyr-sdk-1.0.1"),
    [string]$Version = "1.0.1"
)

$ErrorActionPreference = "Stop"
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "     Zephyr SDK $Version Installer (RISC-V Toolchain)       " -ForegroundColor Yellow
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host " Target Directory: $InstallDir"

$ParentDir = Split-Path -Parent $InstallDir
if (-not (Test-Path $ParentDir)) {
    New-Item -ItemType Directory -Force -Path $ParentDir | Out-Null
}

$TempDir = Join-Path $PSScriptRoot "..\.cache"
if (-not (Test-Path $TempDir)) {
    New-Item -ItemType Directory -Force -Path $TempDir | Out-Null
}

$BaseUrl = "https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v$Version"
$MinimalArchive = "zephyr-sdk-${Version}_windows-x86_64_minimal.7z"
$RiscvArchive = if ($Version.StartsWith("0.")) { 
    "toolchain_windows-x86_64_riscv64-zephyr-elf.7z" 
} else { 
    "toolchain_gnu_windows-x86_64_riscv64-zephyr-elf.7z" 
}

$MinimalPath = Join-Path $TempDir $MinimalArchive
$RiscvPath = Join-Path $TempDir $RiscvArchive

# Helper function to download file with progress using Python if available
function Download-FileWithProgress([string]$url, [string]$destination) {
    & "$PSScriptRoot\..\.venv\Scripts\python.exe" "$PSScriptRoot\download_file.py" $url $destination
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to download $url"
    }
}

# Step 1: Download Minimal SDK
if (-not (Test-Path $MinimalPath)) {
    Write-Host "[1/4] Downloading $MinimalArchive (~100 MB)..." -ForegroundColor Green
    $Uri = "$BaseUrl/$MinimalArchive"
    Download-FileWithProgress $Uri $MinimalPath
} else {
    Write-Host "[1/4] Minimal bundle archive already downloaded: $MinimalPath" -ForegroundColor Gray
}

# Step 2: Download RISC-V Toolchain (ESP32-C6 target)
if (-not (Test-Path $RiscvPath)) {
    Write-Host "[2/4] Downloading $RiscvArchive (~125 MB)..." -ForegroundColor Green
    $Uri = "$BaseUrl/$RiscvArchive"
    Download-FileWithProgress $Uri $RiscvPath
} else {
    Write-Host "[2/4] RISC-V toolchain archive already downloaded: $RiscvPath" -ForegroundColor Gray
}

# Step 3: Extract archives
Write-Host "[3/4] Extracting SDK files to $InstallDir..." -ForegroundColor Green
Write-Host "  -> Extracting minimal host tools into $ParentDir..."
tar.exe -xf $MinimalPath -C $ParentDir

$ToolchainDir = if ($Version.StartsWith("0.")) { $InstallDir } else { (Join-Path $InstallDir "gnu") }
if (-not (Test-Path $ToolchainDir)) {
    New-Item -ItemType Directory -Force -Path $ToolchainDir | Out-Null
}

Write-Host "  -> Extracting RISC-V 64 toolchain into $ToolchainDir..."
tar.exe -xf $RiscvPath -C $ToolchainDir

# Step 4: Register Zephyr SDK with CMake
$ExportScript = Join-Path $InstallDir "cmake\zephyr_sdk_export.cmake"
if (Test-Path $ExportScript) {
    Write-Host "[4/4] Registering Zephyr SDK with CMake..." -ForegroundColor Green
    Push-Location $InstallDir
    try {
        & "$PSScriptRoot\..\.venv\Scripts\cmake.exe" -P "cmake\zephyr_sdk_export.cmake"
    } finally {
        Pop-Location
    }
}

# Set Environment Variable
$env:ZEPHYR_SDK_INSTALL_DIR = $InstallDir
[Environment]::SetEnvironmentVariable("ZEPHYR_SDK_INSTALL_DIR", $InstallDir, "User")

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host " Zephyr SDK $Version (RISC-V) installed successfully!" -ForegroundColor Green
Write-Host " ZEPHYR_SDK_INSTALL_DIR set to: $InstallDir" -ForegroundColor White
Write-Host "============================================================" -ForegroundColor Cyan
