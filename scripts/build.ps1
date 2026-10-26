# scripts/build.ps1 - Convenience builder for Seeed Studio XIAO ESP32C6 labs
[CmdletBinding()]
param(
    [Parameter(Position = 0)]
    [string]$Lab = "labs\01_blinky",

    [switch]$Pristine,
    [switch]$Menuconfig,
    [switch]$Guiconfig,
    [string]$Board = "xiao_esp32c6",
    [string]$ExtraArgs = ""
)

$WorkspaceRoot = (Resolve-Path "$PSScriptRoot\..").Path
. "$PSScriptRoot\env.ps1"

# Resolve Lab path
if (-not (Test-Path $Lab)) {
    $altPath = Join-Path $WorkspaceRoot "labs\$Lab"
    if (Test-Path $altPath) {
        $Lab = $altPath
    } else {
        Write-Error "Lab folder '$Lab' not found! Available labs: $(Get-ChildItem -Directory "$WorkspaceRoot\labs" | Select-Object -ExpandProperty Name)"
        exit 1
    }
}

$pristineFlag = if ($Pristine) { "-p always" } else { "-p auto" }
$targetFlag = ""
if ($Menuconfig) { $targetFlag = "-t menuconfig" }
if ($Guiconfig) { $targetFlag = "-t guiconfig" }

Write-Host ">>> Building '$Lab' for Board: $Board..." -ForegroundColor Cyan

# Ensure BOARD_ROOT includes workspace boards directory
$cmd = "west build -b $Board `"$Lab`" $pristineFlag $targetFlag -- -DBOARD_ROOT=`"$WorkspaceRoot`" $ExtraArgs"
Write-Host "Running: $cmd" -ForegroundColor Gray

Invoke-Expression $cmd
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed with exit code $LASTEXITCODE."
    exit $LASTEXITCODE
}

Write-Host "`n>>> Build Successful! Output binary located in: build\zephyr\zephyr.bin" -ForegroundColor Green
Write-Host ">>> To flash to your XIAO ESP32C6: .\scripts\flash.ps1" -ForegroundColor Yellow
