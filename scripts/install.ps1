[CmdletBinding()]
param(
    [string]$Prefix = "",
    [switch]$NoPath,
    [switch]$Force
)

$ErrorActionPreference = "Stop"
$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path

Push-Location $RepoRoot
try {

# Default prefix
if (-not $Prefix) {
    if ($IsWindows -or $env:OS -eq "Windows_NT") {
        $Prefix = Join-Path $env:LOCALAPPDATA "Programs\PromptLib"
    } elseif ($IsMacOS -or $IsLinux) {
        $homeLocal = Join-Path $env:HOME ".local\bin"
        if (Test-Path $homeLocal) {
            $Prefix = $homeLocal
        } else {
            $Prefix = Join-Path $env:HOME ".local\bin"
        }
    } else {
        $Prefix = Join-Path $env:HOME ".local\bin"
    }
}

$BinaryName = if ($IsWindows -or $env:OS -eq "Windows_NT") { "pp.exe" } else { "pp" }

# Find source binary
$BinarySrc = Join-Path $RepoRoot "build\ninja-release\bin\$BinaryName"
if (-not (Test-Path $BinarySrc)) {
    $BinarySrc = Join-Path $RepoRoot "build\ninja-debug\bin\$BinaryName"
}
if (-not (Test-Path $BinarySrc)) {
    Write-Host "ERROR: Could not find pp binary." -ForegroundColor Red
    Write-Host "Run .\scripts\release.ps1 first, or build with:" -ForegroundColor Red
    Write-Host "  cmake --preset ninja-release; cmake --build --preset ninja-release" -ForegroundColor Red
    exit 1
}

Write-Host "=== PromptLib Install ==="
Write-Host "  Source:  $BinarySrc"
Write-Host "  Prefix:  $Prefix"
Write-Host ""

# Check for existing binary
$Dest = Join-Path $Prefix $BinaryName
if ((Test-Path $Dest) -and (-not $Force)) {
    $answer = Read-Host "Binary already exists at $Dest. Overwrite? [y/N]"
    if ($answer -notmatch '^[Yy]') {
        Write-Host "Installation cancelled."
        exit 0
    }
}

# Create destination directory
if (-not (Test-Path $Prefix)) {
    New-Item -ItemType Directory -Path $Prefix -Force | Out-Null
}

# Copy binary
Copy-Item $BinarySrc $Dest -Force
Write-Host "Installed: $Dest"

# Verify
Write-Host ""
Write-Host "--- Verification ---"
& $Dest --version

# PATH setup
if (-not $NoPath) {
    Write-Host ""
    Write-Host "--- PATH Setup ---"

    $currentUserPath = [Environment]::GetEnvironmentVariable("Path", "User")
    $currentMachinePath = [Environment]::GetEnvironmentVariable("Path", "Machine")
    $allPaths = "$currentUserPath;$currentMachinePath"

    if ($allPaths -split ';' | Where-Object { $_ -eq $Prefix }) {
        Write-Host "$Prefix is already on your PATH."
    } else {
        Write-Host "The install directory is not on your PATH."
        $answer = Read-Host "Add $Prefix to your user PATH? [y/N]"
        if ($answer -match '^[Yy]') {
            try {
                $newPath = if ($currentUserPath) {
                    "$currentUserPath;$Prefix"
                } else {
                    $Prefix
                }
                [Environment]::SetEnvironmentVariable("Path", $newPath, "User")
                Write-Host "Added to user PATH."
                Write-Host "Note: Restart your terminal for the change to take effect."
            } catch {
                Write-Host "Could not update PATH automatically." -ForegroundColor Yellow
                Write-Host "Add the following directory to your PATH manually:" -ForegroundColor Yellow
                Write-Host "  $Prefix" -ForegroundColor Yellow
            }
        } else {
            Write-Host "Skipped. To add it manually, add the following to your PATH:"
            Write-Host "  $Prefix"
        }
    }
}

# If on Unix-like, check shell profile
if (-not ($IsWindows -or $env:OS -eq "Windows_NT")) {
    Write-Host ""
    Write-Host "To ensure the install directory is on your PATH in new shells,"
    Write-Host "add the following to your shell profile (~/.bashrc, ~/.zshrc, etc.):"
    Write-Host ""
    Write-Host "  export PATH=`"$Prefix`":`$PATH`""
}

Write-Host ""
Write-Host "=== Install complete ==="

} finally {
    Pop-Location
}
