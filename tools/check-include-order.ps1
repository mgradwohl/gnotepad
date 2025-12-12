# Check include order in source files according to project guidelines
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

# Function to categorize an include
# Returns: 1=MatchingHeader, 2=Project, 3=ThirdParty, 4=Qt, 5=Std, 6=Other
function Get-IncludeCategory {
    param(
        [string]$includeLine,
        [string]$matchingHeader
    )
    
    # Extract include path
    if ($includeLine -match '#include\s+[<"]([^>"]*)[>"]') {
        $includePath = $Matches[1]
        
        # 1. Matching header
        if ($matchingHeader -and ($includePath -eq $matchingHeader -or $includePath -like "*/$matchingHeader")) {
            return 1
        }
        
        # 2. Project headers
        if ($includePath -match '^(src/|include/|tests/|ui/|app/)') {
            return 2
        }
        
        # 3. Third-party non-Qt
        if ($includePath -match '^(spdlog/|fmt/|boost/)') {
            return 3
        }
        
        # 4. Qt headers
        if ($includePath -match '^(Qt|q)' -or $includePath -match '/q') {
            return 4
        }
        
        # 5. C++ standard library (no slashes)
        if ($includePath -notmatch '/') {
            return 5
        }
        
        # 6. Other
        return 6
    }
    return 6
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
    
    $content = Get-Content $file -ErrorAction SilentlyContinue
    if (-not $content) { return $true }
    
    foreach ($line in $content) {
        $lineNum++
        
        # Skip empty lines and comments
        if ([string]::IsNullOrWhiteSpace($line) -or $line -match '^\s*//') {
            continue
        }
        
        # Check if this is an include line
        if ($line -match '^\s*#include') {
            $category = Get-IncludeCategory $line $matchingHeader
            
            # Check if category order is violated
            if ($category -lt $lastCategory) {
                Write-Host "  Line ${lineNum}: Include order violation"
                Write-Host "    Category $category comes after category $lastCategory"
                Write-Host "    $line"
                $violations = $true
            }
            
            $lastCategory = $category
        }
    }
    
    return -not $violations
}

Push-Location $projectRoot
try {
    # Find all .cpp files in src directory
    $violationsFound = $false
    $files = Get-ChildItem -Path "src" -Recurse -Filter "*.cpp" |
             Where-Object { $_.FullName -notmatch "\\build\\" -and $_.FullName -notmatch "\\.git\\" }

    foreach ($file in $files) {
        if (-not (Test-FileIncludes $file.FullName)) {
            Write-Host "Include order violation in: $($file.FullName)" -ForegroundColor Yellow
            $violationsFound = $true
        }
    }

    if ($violationsFound) {
        Write-Host ""
        Write-Host "Include order violations found!" -ForegroundColor Red
        Write-Host "Please review the include order guidelines in docs/STATIC_ANALYSIS.md"
        exit 1
    } else {
        Write-Host "All files follow include order guidelines." -ForegroundColor Green
        exit 0
    }
} finally {
    Pop-Location
}
