# Copyright (c) 2025. All rights reserved.
# Stages a clean Windows publish tree in build/win-publish with only runtime assets.
Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$sourceRoot = Join-Path $repoRoot "build/win-optimized"
$publishRoot = Join-Path $repoRoot "build/win-publish"

if (-not (Test-Path $sourceRoot)) {
    throw "Source build not found at $sourceRoot. Run 'Build Optimized (Windows)' first."
}

if (Test-Path $publishRoot) {
    Remove-Item -Path $publishRoot -Recurse -Force
}

New-Item -ItemType Directory -Path $publishRoot | Out-Null

$rootFiles = @(
    'GnotePad.exe',
    'qt.conf',
    'Qt6Core.dll',
    'Qt6Gui.dll',
    'Qt6Widgets.dll',
    'Qt6PrintSupport.dll',
    'Qt6Svg.dll',
    'Qt6SvgWidgets.dll',
    'brotlicommon.dll',
    'brotlidec.dll',
    'bz2.dll',
    'double-conversion.dll',
    'freetype.dll',
    'harfbuzz.dll',
    'icudt78.dll',
    'icuin78.dll',
    'icuio78.dll',
    'icutu78.dll',
    'icuuc78.dll',
    'jpeg62.dll',
    'libexpat.dll',
    'libpng16.dll',
    'pcre2-16.dll',
    'zlib1.dll',
    'zstd.dll'
)

$pluginFiles = @{
    'platforms'    = @('qwindows.dll');
    'imageformats' = @('qico.dll', 'qjpeg.dll', 'qsvg.dll');
    'iconengines'  = @('qsvgicon.dll');
    'styles'       = @('qmodernwindowsstyle.dll');
}

function Copy-RequiredFile {
    param (
        [string]$SourcePath,
        [string]$Destination
    )

    if (-not (Test-Path $SourcePath)) {
        throw "Required file missing: $SourcePath"
    }

    Copy-Item -Path $SourcePath -Destination $Destination -Force
}

foreach ($file in $rootFiles) {
    Copy-RequiredFile -SourcePath (Join-Path $sourceRoot $file) -Destination $publishRoot
}

$pluginsRoot = Join-Path $publishRoot 'plugins'
foreach ($pluginFolder in $pluginFiles.Keys) {
    $targetDir = Join-Path $pluginsRoot $pluginFolder
    New-Item -ItemType Directory -Path $targetDir -Force | Out-Null

    $sourceDir = Join-Path $sourceRoot (Join-Path 'plugins' $pluginFolder)
    foreach ($pluginFile in $pluginFiles[$pluginFolder]) {
        Copy-RequiredFile -SourcePath (Join-Path $sourceDir $pluginFile) -Destination $targetDir
    }
}

Write-Host "Publish folder staged at $publishRoot"

# Run tests against the staged runtime
$env:Path = "$publishRoot;$env:Path"
$env:QT_PLUGIN_PATH = "$publishRoot/plugins"
Push-Location $repoRoot
try {
    & ctest --test-dir "$sourceRoot"
    if ($LASTEXITCODE -ne 0) {
        throw "ctest failed with exit code $LASTEXITCODE"
    }
} finally {
    Pop-Location
}

# Create distributable zip if tests pass
$distRoot = Join-Path $repoRoot "packaging/dist"
if (-not (Test-Path $distRoot)) {
    New-Item -ItemType Directory -Path $distRoot | Out-Null
}

$version = $env:GNOTE_VERSION
if ([string]::IsNullOrWhiteSpace($version)) {
    # Extract version from CMakeLists.txt
    $cmakeFile = Join-Path $repoRoot "CMakeLists.txt"
    $versionMatch = Select-String -Path $cmakeFile -Pattern 'project\s*\(\s*GnotePad\s+VERSION\s+([0-9.]+)' | Select-Object -First 1
    if ($versionMatch) {
        $version = $versionMatch.Matches[0].Groups[1].Value
    } else {
        $version = "0.0.0"
        Write-Warning "Could not extract version from CMakeLists.txt, using $version"
    }
}
$zipPath = Join-Path $distRoot "GnotePad-$version-windows.zip"
if (Test-Path $zipPath) {
    Remove-Item $zipPath -Force
}
Compress-Archive -Path (Join-Path $publishRoot '*') -DestinationPath $zipPath -CompressionLevel Optimal
Write-Host "Zip created at $zipPath"
