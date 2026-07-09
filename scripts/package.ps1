[CmdletBinding()]
param(
    [string]$Version = "",
    [string]$DistDir = "",
    [switch]$SkipBuild
)

<#
.SYNOPSIS
    Builds a graphical Windows installer (.exe) from the release binary.

.DESCRIPTION
    Takes the pp.exe produced by scripts/release.ps1 and wraps it in an
    Inno Setup graphical installer (pp-setup-<version>.exe).

    Prerequisites: Inno Setup (https://jrsoftware.org/isinfo.php)

    Usage:
      .\scripts\package.ps1                  # use dist/pp.exe
      .\scripts\package.ps1 -Version 0.2.0   # explicit version
#>

$ErrorActionPreference = "Stop"
$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path

Push-Location $RepoRoot
try {

# Resolve paths
if (-not $DistDir) {
    $DistDir = Join-Path $RepoRoot "dist"
}
$Binary = Join-Path $DistDir "pp.exe"

# Read version
if (-not $Version) {
    $cmakeContent = Get-Content "CMakeLists.txt" -Raw
    if ($cmakeContent -match 'project\(PromptLib\s+VERSION\s+([0-9.]+)') {
        $Version = $Matches[1]
    } else {
        Write-Host "ERROR: Could not determine version from CMakeLists.txt." -ForegroundColor Red
        Write-Host "Pass -Version X.Y.Z to specify manually." -ForegroundColor Red
        exit 1
    }
}

Write-Host "=== Build Windows Installer ==="
Write-Host "  Version:  $Version"
Write-Host "  Binary:   $Binary"
Write-Host ""

# Ensure binary exists
if (-not (Test-Path $Binary)) {
    if (-not $SkipBuild) {
        Write-Host "Binary not found. Running release build..."
        $releaseScript = Join-Path $PSScriptRoot "release.ps1"
        & $releaseScript -Version $Version
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    } else {
        Write-Host "ERROR: Binary not found at $Binary" -ForegroundColor Red
        Write-Host "Run .\scripts\release.ps1 first, or omit -SkipBuild." -ForegroundColor Red
        exit 1
    }
}

# Find Inno Setup compiler
$iscc = $null
$isccPaths = @(
    "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe",
    "${env:ProgramFiles}\Inno Setup 6\ISCC.exe",
    "${env:LOCALAPPDATA}\Programs\Inno Setup 6\ISCC.exe"
)
foreach ($p in $isccPaths) {
    if (Test-Path $p) { $iscc = $p; break }
}
if (-not $iscc) {
    $cmd = Get-Command iscc -ErrorAction SilentlyContinue
    if ($cmd) { $iscc = $cmd.Source }
}

if (-not $iscc) {
    Write-Host "ERROR: Inno Setup (ISCC.exe) not found." -ForegroundColor Red
    Write-Host ""
    Write-Host "Install Inno Setup from: https://jrsoftware.org/isinfo.php" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Quick install via winget:" -ForegroundColor Yellow
    Write-Host "  winget install --id=JRSoftware.InnoSetup -e" -ForegroundColor Yellow
    exit 1
}
Write-Host "  ISCC:     $iscc"

# Invoke Inno Setup
$issFile = Join-Path $RepoRoot "scripts\installer.iss"
Write-Host "  Script:   $issFile"
Write-Host ""

Write-Host "--- Compiling installer ---"
$args = @("/DAppVersion=$Version", $issFile)
& $iscc @args
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Inno Setup compilation failed." -ForegroundColor Red
    exit $LASTEXITCODE
}

$setupExe = Join-Path $DistDir "pp-setup-${Version}.exe"
if (Test-Path $setupExe) {
    $size = [math]::Round((Get-Item $setupExe).Length / 1MB, 1)
    Write-Host ""
    Write-Host "=== Installer ready ==="
    Write-Host "  $setupExe  (${size} MB)"
    Write-Host ""
    Write-Host "Upload this file to your GitHub Release."
} else {
    Write-Host "ERROR: Installer was not produced." -ForegroundColor Red
    exit 1
}

} finally {
    Pop-Location
}
