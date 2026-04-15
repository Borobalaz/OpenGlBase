# Debug/Run script for Connectomics-Imaging Qt host
# Usage: .\debug.ps1 [Debug|Release]

param(
    [string]$Config = "Release"
)

$ErrorActionPreference = "Stop"

$TargetName = "app_qt.exe"
$BuildDir = "build\$Config"
$ExePath = ".\$BuildDir\$TargetName"

if (-not (Test-Path $ExePath)) {
    Write-Host "Executable not found: $ExePath" -ForegroundColor Red
    Write-Host "Build the project first with .\build.ps1 $Config" -ForegroundColor Yellow
    exit 1
}

Write-Host "=== Copying vcpkg dependencies ===" -ForegroundColor Green

$VcpkgBinPath = "C:\vcpkg\installed\x64-windows\bin"

if (Test-Path $VcpkgBinPath) {
    Copy-Item "$VcpkgBinPath\*.dll" -Destination $BuildDir -Force -ErrorAction SilentlyContinue
    Write-Host "DLLs copied to build directory" -ForegroundColor Cyan
} else {
    Write-Host "Warning: vcpkg bin directory not found at $VcpkgBinPath" -ForegroundColor Yellow
}

Write-Host "`n=== Deploying Qt runtime ===" -ForegroundColor Green

$QtRoot = ""
if ($env:QT_ROOT -and (Test-Path $env:QT_ROOT)) {
    $QtRoot = $env:QT_ROOT
} else {
    $QtCandidates = @(
        "C:/Qt/6.11.0/msvc2022_64",
        "C:/Qt/6.10.0/msvc2022_64",
        "C:/Qt/6.9.0/msvc2022_64",
        "C:/Qt/6.8.0/msvc2022_64",
        "C:/Qt/6.7.3/msvc2022_64",
        "C:/Qt/6.7.2/msvc2022_64",
        "C:/Qt/6.7.1/msvc2022_64",
        "C:/Qt/6.7.0/msvc2022_64"
    )

    foreach ($Candidate in $QtCandidates) {
        if (Test-Path (Join-Path $Candidate "bin/windeployqt.exe")) {
            $QtRoot = $Candidate
            break
        }
    }
}

if (-not $QtRoot) {
    Write-Host "Warning: Could not locate Qt runtime root. Set QT_ROOT to enable automatic Qt deployment." -ForegroundColor Yellow
} else {
    $DeployTool = Join-Path $QtRoot "bin/windeployqt.exe"
    if (Test-Path $DeployTool) {
        & $DeployTool --no-translations --no-opengl-sw --dir $BuildDir $ExePath
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Warning: windeployqt failed (code: $LASTEXITCODE)." -ForegroundColor Yellow
        } else {
            Write-Host "Qt runtime deployed to $BuildDir" -ForegroundColor Cyan
        }
    } else {
        Write-Host "Warning: windeployqt not found at $DeployTool" -ForegroundColor Yellow
    }
}

Write-Host "`n=== Running application ===" -ForegroundColor Green
& $ExePath

if ($LASTEXITCODE -ne 0) {
    Write-Host "`nApplication exited with code: $LASTEXITCODE" -ForegroundColor Yellow
}
