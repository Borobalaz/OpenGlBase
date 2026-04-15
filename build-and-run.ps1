# Build and run script (Qt host convenience wrapper)
# Usage: .\build-and-run.ps1 [Debug|Release]

param(
    [string]$Config = "Release"
)

$ErrorActionPreference = "Stop"

Write-Host "=== Building and Running ===" -ForegroundColor Cyan

.\build.ps1 $Config
if ($LASTEXITCODE -ne 0) { exit 1 }

.\debug.ps1 $Config
