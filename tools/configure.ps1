<#
.SYNOPSIS
    Configure GnotePad for building on Windows.
.DESCRIPTION
    Runs CMake configure with appropriate settings for the specified build type.
.PARAMETER BuildType
    Build type: debug, relwithdebinfo, release, optimized, analyze (default: debug)
.PARAMETER Verbose
    Show verbose output
.EXAMPLE
    .\configure.ps1 debug
.EXAMPLE
    .\configure.ps1 -Verbose optimized
#>
param(
    [Parameter(Position = 0)]
    [ValidateSet("debug", "relwithdebinfo", "release", "optimized", "analyze")]
    [string]$BuildType = "debug",

    [switch]$Verbose
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir

# Validate environment
if (-not $env:LLVM_ROOT) {
    Write-Error "LLVM_ROOT environment variable not set"
    exit 1
}
if (-not $env:VCPKG_ROOT) {
    Write-Error "VCPKG_ROOT environment variable not set"
    exit 1
}

# Paths
$ClangCXX = "$env:LLVM_ROOT\bin\clang++.exe"
$ClangC = "$env:LLVM_ROOT\bin\clang.exe"
$VcpkgToolchain = "$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"

# Build directory (Windows uses win- prefix)
$BuildDir = Join-Path $ProjectRoot "build\win-$BuildType"

# Common CMake args
$CMakeArgs = @(
    "-S", $ProjectRoot
    "-B", $BuildDir
    "-G", "Ninja"
    "-DCMAKE_CXX_COMPILER=$ClangCXX"
    "-DCMAKE_CXX_STANDARD=23"
    "-DCMAKE_CXX_STANDARD_REQUIRED=ON"
    "-DCMAKE_CXX_EXTENSIONS=OFF"
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    "-DCMAKE_TOOLCHAIN_FILE=$VcpkgToolchain"
    "-DCMAKE_EXE_LINKER_FLAGS=-fuse-ld=lld"
    "-DCMAKE_SHARED_LINKER_FLAGS=-fuse-ld=lld"
)

# Add Qt paths if set
if ($env:Qt6_DIR) {
    $CMakeArgs += "-DQt6_DIR=$env:Qt6_DIR"
}
if ($env:QT6_PREFIX_PATH) {
    $CMakeArgs += "-DCMAKE_PREFIX_PATH=$env:QT6_PREFIX_PATH"
}

# Build type specific args
switch ($BuildType) {
    "debug" {
        $CMakeArgs += "-DCMAKE_BUILD_TYPE=Debug"
    }
    "relwithdebinfo" {
        $CMakeArgs += "-DCMAKE_BUILD_TYPE=RelWithDebInfo"
    }
    "release" {
        $CMakeArgs += "-DCMAKE_BUILD_TYPE=Release"
    }
    "optimized" {
        $CMakeArgs += @(
            "-DCMAKE_BUILD_TYPE=Release"
            "-DGNOTE_ENABLE_IPO=ON"
            "-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON"
            "-DCMAKE_EXE_LINKER_FLAGS_RELEASE=-s"
            "-DCMAKE_SHARED_LINKER_FLAGS_RELEASE=-s"
            "-DCMAKE_CXX_FLAGS_RELEASE=-O3 -DNDEBUG -march=x86-64-v3 -fomit-frame-pointer"
        )
    }
    "analyze" {
        $CMakeArgs += @(
            "-DCMAKE_C_COMPILER=$ClangC"
            "-DCMAKE_BUILD_TYPE=Debug"
        )
    }
}

if ($Verbose) {
    Write-Host "Configuring $BuildType build in $BuildDir"
    Write-Host "CMake args:"
    $CMakeArgs | ForEach-Object { Write-Host "  $_" }
}

& cmake $CMakeArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Configuration complete. Run 'tools\build.ps1 $BuildType' to build."
