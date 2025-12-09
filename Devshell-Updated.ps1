# Load Visual Studio DevShell
Import-Module "C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"

Enter-VsDevShell a73be645 -SkipAutomaticLocation -DevCmdArguments "-arch=x64 -host_arch=x64"

# Prepend LLVM
$llvm = "C:\Program Files\LLVM\bin"
if (Test-Path $llvm) {
    $env:PATH = "$llvm;$env:PATH"
    $env:LLVM_ROOT = $llvm
}

# Prepend CMake
$cmake = "C:\Program Files\CMake\bin"
if (Test-Path $cmake) {
    $env:PATH = "$cmake;$env:PATH"
    $env:CMAKE_ROOT = $cmake
}

# Prepend VCPKG
$vcpkg = "${env:USERPROFILE}\source\vcpkg"
if (Test-Path $vcpkg) {
    $env:PATH = "$vcpkg;$env:PATH"
    $env:VCPKG_ROOT = $vcpkg
}

# Qt6 via vcpkg
$qtPrefix = "$vcpkg\installed\x64-windows"
$qtConfig = "$qtPrefix\share\Qt6"
if (Test-Path $qtConfig) {
    $env:QT6_PREFIX_PATH = $qtPrefix
    $env:Qt6_DIR = $qtConfig
}

Write-Host "Using clang from:  $((Get-Command clang++.exe).Source)" -ForegroundColor Green
Write-Host "Using cmake from: $((Get-Command cmake.exe).Source)" -ForegroundColor Green
Write-Host "Using vcpkg from: $((Get-Command vcpkg.exe).Source)" -ForegroundColor Green
if ($env:Qt6_DIR) { Write-Host "Using Qt6 from:  $env:Qt6_DIR" -ForegroundColor Green }