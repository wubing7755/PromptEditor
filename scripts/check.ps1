[CmdletBinding()]
param(
    [string]$Preset = "ninja-debug",
    [switch]$SkipFormat,
    [switch]$EnableTidy
)

$ErrorActionPreference = "Stop"
$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")

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

if (-not $SkipFormat -and (Get-Command clang-format -ErrorAction SilentlyContinue)) {
    $files = Get-ChildItem -Path include,src,tests -Recurse -File -Include *.c,*.h,*.cpp |
        Sort-Object FullName
    if ($files.Count -gt 0) {
        Invoke-CheckedCommand -Command "clang-format" -Arguments @(
            "--dry-run",
            "--Werror",
            $files.FullName
        )
    }
} elseif (-not $SkipFormat) {
    Write-Host "Skipping clang-format: command not found."
}

Invoke-CheckedCommand -Command "cmake" -Arguments @("--preset", $Preset)
Invoke-CheckedCommand -Command "cmake" -Arguments @("--build", "--preset", $Preset)

if ($EnableTidy) {
    if (Get-Command clang-tidy -ErrorAction SilentlyContinue) {
        $buildDirectory = Join-Path "build" $Preset
        $compileCommands = Join-Path $buildDirectory "compile_commands.json"
        $generatedIncludeDirectory = Join-Path $buildDirectory "generated\include"
        $files = Get-ChildItem -Path src,tests -Recurse -File -Include *.c |
            Where-Object { $_.FullName -notlike "*\tests\package_smoke\*" } |
            Where-Object { $_.FullName -notlike "*\tests\subproject_smoke\*" } |
            Sort-Object FullName
        foreach ($file in $files) {
            if (Test-Path $compileCommands) {
                Invoke-CheckedCommand -Command "clang-tidy" -Arguments @(
                    $file.FullName,
                    "--warnings-as-errors=*",
                    "-p",
                    $buildDirectory
                )
            } else {
                Invoke-CheckedCommand -Command "clang-tidy" -Arguments @(
                    $file.FullName,
                    "--warnings-as-errors=*",
                    "--extra-arg=-std=c11",
                    "--",
                    "-Iinclude",
                    "-I$generatedIncludeDirectory"
                )
            }
        }
    } else {
        Write-Host "Skipping clang-tidy: command not found."
    }
}

Invoke-CheckedCommand -Command "ctest" -Arguments @("--preset", $Preset, "--output-on-failure")
} finally {
    Pop-Location
}
