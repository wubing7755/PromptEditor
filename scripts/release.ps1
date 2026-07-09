[CmdletBinding()]
param(
    [string]$Preset = "ninja-release",
    [string]$Version = "",
    [switch]$SkipTests,
    [switch]$Installer
)

$ErrorActionPreference = "Stop"
$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path

Push-Location $RepoRoot
try {

function Invoke-CheckedCommand {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Command,
        [Parameter(Mandatory = $true)]
        [string[]]$Arguments
    )
    & $Command @Arguments
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}

# Determine version from CMakeLists.txt if not provided
if (-not $Version) {
    $cmakeContent = Get-Content "CMakeLists.txt" -Raw
    if ($cmakeContent -match 'project\(PromptLib\s+VERSION\s+([0-9.]+)') {
        $Version = $Matches[1]
    } else {
        $Version = "0.0.0"
    }
}

# Detect platform
if ($IsWindows -or $env:OS -eq "Windows_NT") {
    $Platform = if ([Environment]::Is64BitOperatingSystem) { "windows-x64" } else { "windows-x86" }
} elseif ($IsMacOS) {
    $Platform = "macos"
} elseif ($IsLinux) {
    $Platform = "linux"
} else {
    $Platform = "unknown"
}

$BuildDir = "build\$Preset"
$DistDir = Join-Path $RepoRoot "dist"
$ArchiveName = "pp-${Version}-${Platform}"

Write-Host "=== PromptLib Release Build ==="
Write-Host "  Preset:   $Preset"
Write-Host "  Version:  $Version"
Write-Host "  Platform: $Platform"
Write-Host ""

# 1. Configure
Write-Host "--- Configuring ---"
Invoke-CheckedCommand -Command "cmake" -Arguments @("--preset", $Preset)

# 2. Build
Write-Host ""
Write-Host "--- Building ---"
Invoke-CheckedCommand -Command "cmake" -Arguments @("--build", "--preset", $Preset)

# 3. Test
if (-not $SkipTests) {
    Write-Host ""
    Write-Host "--- Testing ---"
    Invoke-CheckedCommand -Command "ctest" -Arguments @("--preset", $Preset, "--output-on-failure")
} else {
    Write-Host ""
    Write-Host "--- Skipping tests (-SkipTests) ---"
}

# 4. Package
Write-Host ""
Write-Host "--- Packaging ---"
if (Test-Path $DistDir) {
    Remove-Item -Recurse -Force $DistDir
}
New-Item -ItemType Directory -Path $DistDir -Force | Out-Null

$BinarySrc = Join-Path $BuildDir "bin\pp.exe"
if (-not (Test-Path $BinarySrc)) {
    $BinarySrc = Join-Path $BuildDir "bin\pp"
}

$Ext = if ($Platform -like "windows*") { ".exe" } else { "" }
$BinaryDst = Join-Path $DistDir "pp$Ext"
Copy-Item $BinarySrc $BinaryDst
Write-Host "  Binary: dist\pp$Ext"

# Create zip archive
try {
    $zipPath = Join-Path $DistDir "$ArchiveName.zip"
    Compress-Archive -Path $BinaryDst -DestinationPath $zipPath -Force
    Write-Host "  Archive: dist\$ArchiveName.zip"
} catch {
    Write-Host "  Note: Could not create zip archive: $_"
}

# 5. Installer (Inno Setup — Windows only)
if ($Installer) {
    Write-Host ""
    Write-Host "--- Building installer ---"
    $iscc = Get-Command iscc -ErrorAction SilentlyContinue
    if (-not $iscc) {
        $isccPaths = @(
            "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe",
            "${env:ProgramFiles}\Inno Setup 6\ISCC.exe"
        )
        foreach ($p in $isccPaths) {
            if (Test-Path $p) { $iscc = $p; break }
        }
    }
    if ($iscc) {
        $issFile = Join-Path $RepoRoot "scripts\installer.iss"
        & $iscc "/DAppVersion=$Version" $issFile
        if ($LASTEXITCODE -eq 0) {
            $setupExe = Join-Path $DistDir "pp-setup-${Version}.exe"
            Write-Host "  Installer: dist\pp-setup-${Version}.exe"
        } else {
            Write-Host "  Warning: Inno Setup returned exit code $LASTEXITCODE" -ForegroundColor Yellow
        }
    } else {
        Write-Host "  Skipping: Inno Setup (iscc) not found." -ForegroundColor Yellow
        Write-Host "  Install from https://jrsoftware.org/isinfo.php" -ForegroundColor Yellow
        Write-Host "  Then run: iscc /DAppVersion=$Version scripts\installer.iss" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "=== Release complete ==="
Write-Host "Output:  $DistDir"
if ($Installer) {
    Write-Host "Tip: Run without -Installer to skip the graphical installer step."
} else {
    Write-Host "Tip: Use -Installer to also build pp-setup-<version>.exe (requires Inno Setup)."
}

} finally {
    Pop-Location
}
