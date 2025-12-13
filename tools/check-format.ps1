<#
.SYNOPSIS
    Check if all source files adhere to clang-format rules.
.DESCRIPTION
    Validates formatting without modifying files. Use clang-format.ps1 to fix violations.
    Uses .clang-format configuration from project root.
.PARAMETER ShowDetails
    Show per-file progress
.EXAMPLE
    .\check-format.ps1
.EXAMPLE
    .\check-format.ps1 -ShowDetails
#>
[CmdletBinding()]
param(
    [switch]$ShowDetails
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Resolve-Path (Join-Path $scriptDir "..")

# Find clang-format executable
$clangFormat = $null
$candidates = @(
    "$env:LLVM_ROOT\bin\clang-format.exe",
    "C:\Program Files\LLVM\bin\clang-format.exe",
    "clang-format"
)

foreach ($cmd in $candidates) {
    try {
        $resolved = Get-Command $cmd -ErrorAction Stop
        $clangFormat = $resolved.Source
        break
    } catch {
        continue
    }
}

if (-not $clangFormat) {
    Write-Error "Error: clang-format not found. Please install LLVM."
    exit 1
}

if ($ShowDetails) {
    Write-Host "Using: $clangFormat"
}
Write-Host "Checking code formatting..."

Push-Location $projectRoot
try {
    # Find all .cpp and .h files in src and tests directories
    $violationsFound = $false
    $files = Get-ChildItem -Path "src", "tests" -Recurse -Include "*.cpp", "*.h" | 
             Where-Object { $_.FullName -notmatch "\\build\\" -and $_.FullName -notmatch "\\.git\\" }

    $fileCount = ($files | Measure-Object).Count
    $checkedCount = 0

    foreach ($file in $files) {
        $checkedCount++
        if ($ShowDetails) {
            Write-Host "[$checkedCount/$fileCount] Checking: $($file.Name)"
        }
        # Check if file would be modified by clang-format
        $result = & $clangFormat --dry-run -Werror $file.FullName 2>&1
        if ($LASTEXITCODE -ne 0) {
            Write-Host "  Formatting violation: $($file.FullName)" -ForegroundColor Yellow
            $violationsFound = $true
        }
    }

    if ($violationsFound) {
        Write-Host ""
        Write-Host "Code formatting violations found!" -ForegroundColor Red
        Write-Host "Run '.\tools\clang-format.ps1' to fix formatting."
        exit 1
    } else {
        Write-Host "All $fileCount files are properly formatted." -ForegroundColor Green
        exit 0
    }
} finally {
    Pop-Location
}
