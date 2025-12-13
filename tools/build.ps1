<#
.SYNOPSIS
    Build GnotePad on Windows (run configure.ps1 first).
.DESCRIPTION
    Runs CMake build for the specified build type.
.PARAMETER BuildType
    Build type: debug, relwithdebinfo, release, optimized, analyze (default: debug)
.PARAMETER Target
    Build specific target(s)
.PARAMETER Verbose
    Show verbose build output
.EXAMPLE
    .\build.ps1 debug
.EXAMPLE
    .\build.ps1 -Target run-clang-tidy debug
.EXAMPLE
    .\build.ps1 -Verbose optimized
#>
param(
    [Parameter(Position = 0)]
    [ValidateSet("debug", "relwithdebinfo", "release", "optimized", "analyze")]
    [string]$BuildType = "debug",

    [string]$Target = "",

    [switch]$Verbose
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir

# Build directory
$BuildDir = Join-Path $ProjectRoot "build\win-$BuildType"

# Check if configured
$NinjaFile = Join-Path $BuildDir "build.ninja"
if (-not (Test-Path $NinjaFile)) {
    Write-Error "Build directory '$BuildDir' not configured. Run 'tools\configure.ps1 $BuildType' first."
    exit 1
}

# Build arguments
$CMakeArgs = @("--build", $BuildDir)

if ($Target) {
    $CMakeArgs += @("--target", $Target)
}

if ($Verbose) {
    $CMakeArgs += "--verbose"
    Write-Host "Building $BuildType in $BuildDir"
    if ($Target) {
        Write-Host "Target: $Target"
    }
}

& cmake $CMakeArgs
exit $LASTEXITCODE
