<#
.SYNOPSIS
    Run Clang Static Analyzer (scan-build) on GnotePad (Windows).
.DESCRIPTION
    Configures a separate 'analyze' build and runs scan-build.
.PARAMETER ReportDir
    Output directory for reports (default: scan-build-report)
.PARAMETER ShowDetails
    Show verbose output
.EXAMPLE
    .\run-scan-build.ps1
.EXAMPLE
    .\run-scan-build.ps1 -ReportDir "my-report"
#>
[CmdletBinding()]
param(
    [string]$ReportDir = "scan-build-report",
    [switch]$ShowDetails
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir

# Validate environment
if (-not $env:LLVM_ROOT) {
    Write-Error "LLVM_ROOT environment variable not set"
    exit 1
}

# Paths
$ClangC = "$env:LLVM_ROOT\bin\clang.exe"
$ClangCXX = "$env:LLVM_ROOT\bin\clang++.exe"
$ScanBuild = "$env:LLVM_ROOT\bin\scan-build.bat"

if (-not (Test-Path $ScanBuild)) {
    Write-Error "scan-build.bat not found at $ScanBuild"
    exit 1
}

$BuildDir = Join-Path $ProjectRoot "build\win-analyze"
$ReportPath = Join-Path $ProjectRoot $ReportDir

# Configure the analyze build
if ($ShowDetails) {
    Write-Host "Configuring analyze build..."
}
$ConfigArgs = @("analyze")
if ($ShowDetails) { $ConfigArgs = @("-ShowDetails") + $ConfigArgs }
& "$ScriptDir\configure.ps1" @ConfigArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# Run scan-build
if ($ShowDetails) {
    Write-Host "Running scan-build..."
    Write-Host "Report directory: $ReportPath"
}

& $ScanBuild -o $ReportPath --status-bugs `
    --use-cc="$ClangC" --use-c++="$ClangCXX" `
    cmake --build $BuildDir

if ($LASTEXITCODE -ne 0) {
    Write-Host "scan-build found issues. See report in $ReportPath" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "scan-build completed successfully. Report in $ReportPath" -ForegroundColor Green
