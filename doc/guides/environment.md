# Environment Setup

This guide walks through installing the tools this project needs on a new
machine. Follow the section for your platform, then verify everything works.

## Requirements

| Tool | Required | Purpose |
| --- | --- | --- |
| CMake 3.21+ | Yes | Configure and generate build files |
| C11 compiler | Yes | Compile C source (MSVC, GCC, Clang, or AppleClang) |
| Ninja | Yes (default presets) | Build generator for the `ninja-*` presets |
| clangd | No | Editor code navigation and diagnostics |
| clang-format | No | Local formatting checks |
| clang-tidy | No | Optional local static analysis |

The project's bootstrap scripts validate these tools. Run them after setup to
confirm your environment is ready.

## Windows

**Option A (recommended):** Install Visual Studio Build Tools 2022 with the
"Desktop development with C++" workload. Use "Developer PowerShell" to run
build commands (MSVC on PATH). Install CMake and Ninja via winget:

```powershell
winget install Kitware.CMake Ninja-build.Ninja
```

**Option B (MSYS2):** Install [MSYS2](https://www.msys2.org/), launch UCRT64 shell:

```sh
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-{cmake,ninja,gcc}
```

**Option C (WSL):** `wsl --install` from PowerShell, then follow the Linux section.
Clone inside the WSL filesystem for best performance.

## Linux

| Distribution | Install command |
|---|---|
| Debian / Ubuntu | `sudo apt install cmake ninja-build gcc` |
| Fedora / RHEL | `sudo dnf install cmake ninja-build gcc` |
| Arch Linux | `sudo pacman -S cmake ninja gcc` |
| Other | Install `cmake`, `ninja`, and `gcc` or `clang` via your package manager |

To use Clang, substitute `clang` for `gcc`. The bootstrap script reports missing tools.

## macOS

Install the Xcode Command Line Tools for AppleClang:

```sh
xcode-select --install
```

Install Homebrew from [brew.sh](https://brew.sh/), then:

```sh
brew install cmake ninja
```

Optional editor tools:

```sh
brew install llvm
```

Homebrew installs LLVM "keg-only" — it is not placed on PATH by default. Add
the path shown in the Homebrew post-install output, typically:

```sh
export PATH="$(brew --prefix llvm)/bin:$PATH"
```

The bootstrap scripts accept AppleClang from Xcode or Homebrew's Clang.

## Verification

Run from the repository root:

```sh
./scripts/bootstrap.sh        # .\scripts\bootstrap.ps1 on Windows
```

The script checks for required tools and runs configure. On success it generates
`compile_commands.json` for editor integration. Pass `--check-only` / `-CheckOnly`
to skip the configure step.

## Optional Tools For Editor Integration

- **clangd**: with `compile_commands.json` generated, clangd provides code
  completion, go-to-definition, and real-time diagnostics that match the build
  flags.

- **clang-format**: `./scripts/check.sh` (or `.ps1`) runs `clang-format
  --dry-run` over all C source files. Installing it locally lets you catch
  formatting issues before pushing.

- **clang-tidy**: run with `./scripts/check.sh --enable-tidy` (or `.ps1
  -EnableTidy`). It uses the project `.clang-tidy` configuration. CI also runs
  clang-tidy, so local installation is optional.

The recommended VS Code extensions are listed in `.vscode/extensions.json`
(CMake Tools, clangd).

## Next Steps

- **Build and test**: see [cmake.md](cmake.md) for configure, build, test,
  presets, and troubleshooting.
- **Test expectations**: see [testing.md](testing.md) for test layers and
  static analysis.
- **Contributing**: see [../../CONTRIBUTING.md](../../CONTRIBUTING.md) for branch,
  commit, and PR workflow.
- **Project standard**: see [../../C_PROJECT_STANDARD.md](../../C_PROJECT_STANDARD.md)
  for the full engineering standard.
