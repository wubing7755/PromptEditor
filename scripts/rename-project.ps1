[CmdletBinding()]
param(
    [Parameter(Mandatory = $false, Position = 0)]
    [string]$Name
)

$ErrorActionPreference = "Stop"
$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")

Push-Location $RepoRoot
try {

if (-not $Name) {
    Write-Host "Usage: $($MyInvocation.MyCommand.Name) <PascalCaseName>"
    Write-Host ""
    Write-Host "Renames this template project in-place. Run once after cloning."
    Write-Host ""
    Write-Host "Example:"
    Write-Host "  $($MyInvocation.MyCommand.Name) MyNewLib"
    Write-Host ""
    Write-Host "The script derives the lowercase namespace (mylib) and UPPER_CASE"
    Write-Host "macro prefix (MYLIB) from the PascalCase name automatically."
    Write-Host ""
    Write-Host "After renaming, run .\scripts\bootstrap.ps1 to verify the build."
    exit 1
}

$Pascal = $Name

if ($Pascal -notmatch '^[A-Z][A-Za-z0-9]*$') {
    Write-Host "ERROR: project name must start with an uppercase letter and contain only letters and digits." -ForegroundColor Red
    Write-Host "Example: MyNewLib"
    exit 2
}

$Lower = $Pascal.ToLowerInvariant()
$Upper = $Lower.ToUpperInvariant()

Write-Host "Project rename:"
Write-Host "  PascalCase: PromptLib -> $Pascal"
Write-Host "  lowercase:  promptlib -> $Lower"
Write-Host "  UPPER_CASE: PROMPTLIB -> $Upper"
Write-Host ""

$extensions = @('*.c', '*.h', '*.cmake', '*.txt', '*.json', '*.yml', '*.md', '*.in', '*.cpp', '*.ps1', '*.sh')

$scriptPath = $MyInvocation.MyCommand.Path

$files = Get-ChildItem -Recurse -File -Include $extensions -Path . |
    Where-Object {
        $_.FullName -notmatch '[\\/]build[\\/]' -and
        $_.FullName -notmatch '[\\/]\.git[\\/]' -and
        $_.FullName -ne $scriptPath
    } |
    ForEach-Object {
        $content = Get-Content $_.FullName -Raw -ErrorAction SilentlyContinue
        if ($content -match 'PromptLib|promptlib|PROMPTLIB|PromptLib') {
            $_
        }
    }

if (-not $files) {
    Write-Host "No files found with original project identifiers."
    Write-Host "The project may already be renamed."
    exit 0
}

$updated = 0
foreach ($file in $files) {
    $content = Get-Content $file.FullName -Raw
    if (-not $content) { continue }

    # Count occurrences before replacement
    $countPascalStd = ([regex]::Matches($content, 'PromptLib')).Count
    $countLower     = ([regex]::Matches($content, 'promptlib')).Count
    $countUpper     = ([regex]::Matches($content, 'PROMPTLIB')).Count
    $countPascal    = ([regex]::Matches($content, 'PromptLib')).Count

    $newContent = $content `
        -creplace 'PromptLib', $Pascal `
        -creplace 'promptlib', $Lower `
        -creplace 'PROMPTLIB', $Upper `
        -creplace 'PromptLib', $Pascal

    if ($newContent -ne $content) {
        [System.IO.File]::WriteAllText($file.FullName, $newContent, [System.Text.UTF8Encoding]::new($false))
    }

    $relPath = $file.FullName.Substring($RepoRoot.Path.Length + 1)
    Write-Host "  $relPath"
    if ($countPascalStd -gt 0) {
        Write-Host "    PromptLib -> $Pascal ($countPascalStd)"
    }
    if ($countLower -gt 0) {
        Write-Host "    promptlib -> $Lower ($countLower)"
    }
    if ($countUpper -gt 0) {
        Write-Host "    PROMPTLIB -> $Upper ($countUpper)"
    }
    $extraPascal = $countPascal - $countPascalStd
    if ($extraPascal -gt 0) {
        Write-Host "    PromptLib -> $Pascal ($extraPascal)"
    }
    $updated++
}

Write-Host ""
Write-Host "Updated $updated files with new project identifiers."

if (Test-Path "include\promptlib") {
    Move-Item "include\promptlib" "include\$Lower"
    Write-Host "Renamed directory: include\promptlib -> include\$Lower"
}

if (Test-Path "src\promptlib") {
    Move-Item "src\promptlib" "src\$Lower"
    Write-Host "Renamed directory: src\promptlib -> src\$Lower"
}

if (Test-Path "cmake\PromptLibConfig.cmake.in") {
    Move-Item "cmake\PromptLibConfig.cmake.in" "cmake\$Pascal`Config.cmake.in"
    Write-Host "Renamed file: PromptLibConfig.cmake.in -> $Pascal`Config.cmake.in"
}

if (Test-Path "cmake\promptlib_version.h.in") {
    Move-Item "cmake\promptlib_version.h.in" "cmake\$Lower`_version.h.in"
    Write-Host "Renamed file: promptlib_version.h.in -> $Lower`_version.h.in"
}

Write-Host ""
Write-Host "Done. Run .\scripts\bootstrap.ps1 to verify the renamed project builds."

} finally {
    Pop-Location
}
