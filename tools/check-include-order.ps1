<#
.SYNOPSIS
    Check include order in source files according to project guidelines.
.DESCRIPTION
    Validates that #include directives follow the order specified in CONTRIBUTING.md.
.PARAMETER ShowDetails
    Show which files are being checked.
.EXAMPLE
    .\check-include-order.ps1
.EXAMPLE
    .\check-include-order.ps1 -ShowDetails
#>
[CmdletBinding()]
param(
    [switch]$ShowDetails
)

# Check include order in source files according to project guidelines
# See CONTRIBUTING.md#include-guidelines for the authoritative order:
#   1. Matching header (.cpp files only)
#   2. Platform-specific headers in #ifdef guards
#   3. Project headers (src/, ui/, app/, tests/)
#   4. Third-party libraries (spdlog, fmt, boost)
#   5. Qt headers (QtCore/, QSignalBlocker, etc.)
#   6. C++ standard library
#   7. Other
#
# Platform-specific includes within #ifdef guards are allowed at any point
# after the matching header and do not affect category tracking.
#
# Exit with non-zero status if violations are found
Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Resolve-Path (Join-Path $scriptDir "..")

Write-Host "Checking include order in source files..."

# Function to get the expected matching header for a .cpp file
function Get-MatchingHeader {
    param([string]$cppFile)
    $baseName = [System.IO.Path]::GetFileNameWithoutExtension($cppFile)
    return "$baseName.h"
}

# Check if an include path is platform-specific
function Test-PlatformSpecificInclude {
    param([string]$includePath)
    
    # Windows headers
    if ($includePath -match '^(windows\.h|windef\.h|winbase\.h|winuser\.h|wingdi\.h|wincon\.h|shellapi\.h|shlobj\.h|shlwapi\.h|commctrl\.h|commdlg\.h|objbase\.h|ole2\.h|oleauto\.h|oaidl\.h|unknwn\.h|winsock2\.h|ws2tcpip\.h|wincrypt\.h|wininet\.h|winhttp\.h|dbghelp\.h|psapi\.h|tlhelp32\.h)$') {
        return $true
    }
    # Unix/POSIX headers
    if ($includePath -match '^(unistd\.h|sys/|pthread\.h|dlfcn\.h|fcntl\.h|termios\.h|signal\.h|pwd\.h|grp\.h|dirent\.h|poll\.h|mman\.h)') {
        return $true
    }
    # Platform-specific spdlog sinks
    if ($includePath -match '^spdlog/sinks/(msvc_sink|wincolor_sink|syslog_sink|systemd_sink|android_sink)') {
        return $true
    }
    return $false
}

# Function to categorize an include
# Returns: 1=MatchingHeader, 2=PlatformSpecific, 3=Project, 4=ThirdParty, 5=Qt, 6=Std, 7=Other
function Get-IncludeCategory {
    param(
        [string]$includeLine,
        [string]$matchingHeader
    )
    
    # Extract include path and bracket type
    $isQuoted = $includeLine -match '#include\s+"'
    if ($includeLine -match '#include\s+[<"]([^>"]*)[>"]') {
        $includePath = $Matches[1]
        
        # 1. Matching header
        if ($matchingHeader -and ($includePath -eq $matchingHeader -or $includePath -like "*/$matchingHeader")) {
            return 1
        }
        
        # 2. Platform-specific headers
        if (Test-PlatformSpecificInclude $includePath) {
            return 2
        }
        
        # 3. Project headers - explicit paths OR quoted includes (local headers)
        if ($includePath -match '^(src/|include/|tests/|ui/|app/)' -or $isQuoted) {
            return 3
        }
        
        # 4. Third-party non-Qt
        if ($includePath -match '^(spdlog/|fmt/|boost/)') {
            return 4
        }
        
        # 5. Qt headers (QtCore/qfile.h, QSignalBlocker, etc.)
        # ^Q[A-Z] catches Qt classes, ^Qt catches module paths, /q catches paths with Qt headers
        if ($includePath -match '^(Qt|Q[A-Z])' -or $includePath -match '/q') {
            return 5
        }
        
        # 6. C++ standard library (no slashes)
        if ($includePath -notmatch '/') {
            return 6
        }
        
        # 7. Other
        return 7
    }
    return 7
}

# Check a single file's include order
function Test-FileIncludes {
    param([string]$file)
    
    $matchingHeader = ""
    
    # Determine matching header for .cpp files
    if ($file -like "*.cpp") {
        $matchingHeader = Get-MatchingHeader $file
    }
    
    $lastCategory = 0
    $lineNum = 0
    $violations = $false
    $inIfdefBlock = 0
    $ifdefDepth = 0
    
    $content = Get-Content $file -ErrorAction SilentlyContinue
    if (-not $content) { return $true }
    
    foreach ($line in $content) {
        $lineNum++
        
        # Skip empty lines and comments
        if ([string]::IsNullOrWhiteSpace($line) -or $line -match '^\s*//') {
            continue
        }
        
        # Track #ifdef/#endif blocks for platform-specific detection
        if ($line -match '^\s*#if') {
            $ifdefDepth++
            # Check if this is a platform-specific ifdef
            if ($line -match '^\s*#if(def)?\s+((_WIN32|WIN32|_MSC_VER|__linux__|__APPLE__|__unix__|__GNUC__|__clang__|NDEBUG|_DEBUG))') {
                $inIfdefBlock = $ifdefDepth
            }
        }
        elseif ($line -match '^\s*#endif') {
            if ($ifdefDepth -eq $inIfdefBlock) {
                $inIfdefBlock = 0
            }
            $ifdefDepth--
            if ($ifdefDepth -lt 0) { $ifdefDepth = 0 }
        }
        # #else and #elif stay in ifdef block context
        
        # Check if this is an include line
        if ($line -match '^\s*#include') {
            $category = Get-IncludeCategory $line $matchingHeader
            
            # Platform-specific includes (category 2) in #ifdef blocks are always OK
            if ($category -eq 2) {
                if ($inIfdefBlock -gt 0 -or $ifdefDepth -gt 0) {
                    # Platform-specific include in #ifdef block - OK, skip order check
                    continue
                } else {
                    # Platform-specific include NOT in #ifdef block - violation!
                    Write-Host "  Line ${lineNum}: Platform-specific include outside #ifdef guard"
                    Write-Host "    $line"
                    $violations = $true
                    continue
                }
            }
            
            # For non-platform-specific includes, check category order
            # Skip order check if we're in an #ifdef block (allows flexibility for conditional includes)
            if ($inIfdefBlock -eq 0 -and $ifdefDepth -eq 0) {
                if ($category -lt $lastCategory) {
                    Write-Host "  Line ${lineNum}: Include order violation"
                    Write-Host "    Category $category comes after category $lastCategory"
                    Write-Host "    $line"
                    $violations = $true
                }
                $lastCategory = $category
            }
        }
    }
    
    return -not $violations
}

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
        if (-not (Test-FileIncludes $file.FullName)) {
            Write-Host "Include order violation in: $($file.FullName)" -ForegroundColor Yellow
            Write-Host ""
            $violationsFound = $true
        }
    }

    if ($violationsFound) {
        Write-Host "Include order violations found!" -ForegroundColor Red
        Write-Host "Please review the include order guidelines in CONTRIBUTING.md#include-guidelines"
        exit 1
    } else {
        Write-Host "All $fileCount files follow include order guidelines." -ForegroundColor Green
        exit 0
    }
} finally {
    Pop-Location
}
