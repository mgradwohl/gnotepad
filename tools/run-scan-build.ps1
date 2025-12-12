# Run clang static analyzer (scan-build) on the project
# Usage: .\run-scan-build.ps1 [BuildDir] [ReportDir]
Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

param(
    [string]$BuildDir = "build/win-analyze",
    [string]$ReportDir = "scan-build-report"
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Resolve-Path (Join-Path $scriptDir "..")

# Find LLVM tools
$llvmRoot = if ($env:LLVM_ROOT) { $env:LLVM_ROOT } else { "C:\Program Files\LLVM" }
$cc = "$llvmRoot\bin\clang.exe"
$cxx = "$llvmRoot\bin\clang++.exe"
$scanBuild = "$llvmRoot\bin\scan-build.bat"

if (-not (Test-Path $cc)) {
    Write-Error "clang.exe not found at $cc. Please install LLVM or set LLVM_ROOT."
    exit 1
}

if (-not (Test-Path $scanBuild)) {
    Write-Error "scan-build.bat not found at $scanBuild. Please install LLVM with scan-build."
    exit 1
}

# Qt6 paths from environment (set by Devshell scripts)
$qt6Dir = $env:Qt6_DIR
$qt6PrefixPath = $env:QT6_PREFIX_PATH
$vcpkgToolchain = if ($env:VCPKG_ROOT) { "$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake" } else { $null }

if (-not $qt6Dir) {
    Write-Error "Qt6_DIR environment variable not set. Run the Devshell script first."
    exit 1
}

Write-Host "Using LLVM from: $llvmRoot" -ForegroundColor Cyan
Write-Host "Using Qt6 from: $qt6Dir" -ForegroundColor Cyan

Push-Location $projectRoot
try {
    # Configure
    $cmakeArgs = @(
        "-S", ".",
        "-B", $BuildDir,
        "-G", "Ninja",
        "-DCMAKE_C_COMPILER=$cc",
        "-DCMAKE_CXX_COMPILER=$cxx",
        "-DCMAKE_CXX_STANDARD=23",
        "-DCMAKE_CXX_STANDARD_REQUIRED=ON",
        "-DCMAKE_CXX_EXTENSIONS=OFF",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
        "-DQt6_DIR=$qt6Dir",
        "-DCMAKE_PREFIX_PATH=$qt6PrefixPath",
        "-DCMAKE_BUILD_TYPE=Debug",
        "-DCMAKE_EXE_LINKER_FLAGS=-fuse-ld=lld",
        "-DCMAKE_SHARED_LINKER_FLAGS=-fuse-ld=lld"
    )

    if ($vcpkgToolchain -and (Test-Path $vcpkgToolchain)) {
        $cmakeArgs += "-DCMAKE_TOOLCHAIN_FILE=$vcpkgToolchain"
    }

    Write-Host "Configuring build..." -ForegroundColor Yellow
    & cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed with exit code $LASTEXITCODE"
    }

    # Run scan-build
    Write-Host "Running scan-build..." -ForegroundColor Yellow
    & $scanBuild -o $ReportDir --status-bugs `
        --use-cc="$cc" --use-c++="$cxx" `
        cmake --build $BuildDir

    if ($LASTEXITCODE -ne 0) {
        Write-Host "scan-build found issues. See report in $ReportDir" -ForegroundColor Red
        exit $LASTEXITCODE
    }

    Write-Host "scan-build completed successfully. Report in $ReportDir" -ForegroundColor Green
} finally {
    Pop-Location
}
