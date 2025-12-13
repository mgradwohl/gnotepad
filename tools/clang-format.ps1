<#
.SYNOPSIS
    Apply clang-format to GnotePad source files (Windows).
.DESCRIPTION
    Formats all source files in-place. Use check-format.ps1 to check without modifying.
    Uses .clang-format configuration from project root.
.PARAMETER BuildType
    Build type to use (default: debug)
.PARAMETER ShowDetails
    Show verbose output
.EXAMPLE
    .\clang-format.ps1
.EXAMPLE
    .\clang-format.ps1 -ShowDetails
#>
[CmdletBinding()]
param(
    [Parameter(Position = 0)]
    [ValidateSet("debug")]
    [string]$BuildType = "debug",

    [switch]$ShowDetails
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir

# Build directory
$BuildDir = Join-Path $ProjectRoot "build\win-$BuildType"

# Configure if needed
$NinjaFile = Join-Path $BuildDir "build.ninja"
if (-not (Test-Path $NinjaFile)) {
    Write-Host "Build not configured. Running configure.ps1 $BuildType..."
    $ConfigArgs = @($BuildType)
    if ($ShowDetails) { $ConfigArgs = @("-ShowDetails") + $ConfigArgs }
    & "$ScriptDir\configure.ps1" @ConfigArgs
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

if ($ShowDetails) {
    Write-Host "Applying clang-format to source files..."
}

& cmake --build $BuildDir --target run-clang-format
exit $LASTEXITCODE
