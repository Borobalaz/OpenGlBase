# Build script for Connectomics-Imaging project (Qt host)
# Usage: .\build.ps1 [Debug|Release]

param(
    [string]$Config = "Release"
)

$ErrorActionPreference = "Stop"

$Target = "app_qt"

$QtRoot = "D:/DevTools/Qt/6.11.2/msvc2022_64"
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
        if (Test-Path (Join-Path $Candidate "lib/cmake/Qt6/Qt6Config.cmake")) {
            $QtRoot = $Candidate
            break
        }
    }
}

if (-not $QtRoot) {
    Write-Host "Qt6 was not found." -ForegroundColor Red
    Write-Host "Set QT_ROOT, e.g.:`n  `$env:QT_ROOT=\"C:/Qt/6.11.0/msvc2022_64\"" -ForegroundColor Yellow
    exit 1
}

Write-Host "=== Configuring CMake ===" -ForegroundColor Green
$cmakeArgs = @(
    "-S", ".",
    "-B", "build",
    "-DCMAKE_TOOLCHAIN_FILE=D:/DevTools/vcpkg/scripts/buildsystems/vcpkg.cmake",
    "-DCMAKE_BUILD_TYPE=$Config"
)

$QtCmakeDir = (Join-Path $QtRoot "lib/cmake") -replace "\\", "/"
$Qt6Dir = (Join-Path $QtRoot "lib/cmake/Qt6") -replace "\\", "/"
$cmakeArgs += "-DCMAKE_PREFIX_PATH=$QtCmakeDir"
$cmakeArgs += "-DQt6_DIR=$Qt6Dir"
Write-Host "Using Qt from: $QtRoot" -ForegroundColor Cyan

& cmake @cmakeArgs

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    exit 1
}

Write-Host "`n=== Building the project ===" -ForegroundColor Green
cmake --build build --config $Config --target $Target

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}

Write-Host "`n=== Build completed successfully ===" -ForegroundColor Green
if (-not (Test-Path "build\$Config\engine.dll")) {
    Write-Host "Engine DLL was not produced: build\$Config\engine.dll" -ForegroundColor Red
    exit 1
}
Write-Host "Executable: build\$Config\$Target.exe" -ForegroundColor Cyan
Write-Host "Engine DLL: build\$Config\engine.dll" -ForegroundColor Cyan
