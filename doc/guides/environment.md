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

Pick one of the three paths below. Option A (MSVC) is the most tested path in
CI. Option B (MSYS2/UCRT64) provides a Unix-like shell and GCC/Clang. Option C
(WSL) delegates to a Linux distribution.

### Option A: Visual Studio Build Tools (MSVC)

Install **Visual Studio Build Tools 2022** or **Visual Studio 2022** from the
[Visual Studio downloads page](https://visualstudio.microsoft.com/downloads/).
During installation, select the **Desktop development with C++** workload. This
provides the MSVC compiler (`cl.exe`), the Windows SDK, and optional CMake and
Ninja if you enable them in the installer.

If you prefer command-line package management, use winget:

```powershell
winget install Kitware.CMake
winget install Ninja-build.Ninja
winget install LLVM.LLVM
```

The LLVM package includes clangd, clang-format, and clang-tidy.

**Important:** A normal PowerShell or Command Prompt does not expose `cl.exe`.
Launch **Developer PowerShell for VS 2022** or **Developer Command Prompt for
VS 2022** from the Start Menu before running any build commands. Alternatively,
use a VS Code CMake kit that initializes the MSVC environment.

### Option B: MSYS2 / UCRT64 (GCC or Clang)

Download and install MSYS2 from [msys2.org](https://www.msys2.org/). Accept
the default installation path.

Launch **MSYS2 UCRT64** from the Start Menu. This opens a shell configured for
the Universal C Runtime — the modern Windows C runtime compatible with standard
C11.

Update the package database first:

```sh
pacman -Syu
```

Re-launch the UCRT64 shell if the update requires it, then install the build
tools:

```sh
pacman -S mingw-w64-ucrt-x86_64-cmake
pacman -S mingw-w64-ucrt-x86_64-ninja
pacman -S mingw-w64-ucrt-x86_64-gcc
```

To use Clang instead of GCC:

```sh
pacman -S mingw-w64-ucrt-x86_64-clang
```

Optional editor tools (clangd, clang-format, clang-tidy):

```sh
pacman -S mingw-w64-ucrt-x86_64-clang-tools-extra
```

All tools are available on PATH inside the UCRT64 shell. Git operations work
from this shell as well, since MSYS2 bundles git.

This option produces native Windows binaries through a Unix-like package
manager and shell environment.

### Option C: WSL (Windows Subsystem for Linux)

From an elevated PowerShell or Command Prompt:

```powershell
wsl --install
```

This installs WSL 2 and a default Ubuntu distribution. Reboot if prompted, then
launch the installed Linux distribution from the Start Menu. From this point,
follow the **Linux** section below.

Clone the repository inside the WSL filesystem (`/home/<user>/...`) for the
best build performance. VS Code users should install the **Remote - WSL**
extension to edit files inside WSL from the Windows VS Code UI.

## Linux

Use your distribution's package manager.

### Debian / Ubuntu

```sh
sudo apt update
sudo apt install cmake ninja-build gcc
```

To use Clang instead of GCC:

```sh
sudo apt install cmake ninja-build clang
```

Optional editor tools:

```sh
sudo apt install clangd clang-format clang-tidy
```

On older Ubuntu releases where CMake 3.21+ is not in the default repositories,
install CMake via pip (`pip install cmake`) or add the
[Kitware APT repository](https://apt.kitware.com/).

### Fedora / RHEL

```sh
sudo dnf install cmake ninja-build gcc
```

Or with Clang:

```sh
sudo dnf install cmake ninja-build clang
```

Optional editor tools:

```sh
sudo dnf install clang-tools-extra
```

### Arch Linux

```sh
sudo pacman -S cmake ninja gcc
```

Or with Clang:

```sh
sudo pacman -S cmake ninja clang
```

Optional editor tools:

```sh
sudo pacman -S clang
```

The `clang` package on Arch bundles clangd, clang-format, and clang-tidy.

### Other distributions

Use your package manager to install **cmake**, **ninja**, and **gcc** or
**clang**. The bootstrap script reports which tools are missing.

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

From the repository root, run the bootstrap script:

**Linux / macOS / MSYS2 UCRT64:**

```sh
./scripts/bootstrap.sh
```

**Windows PowerShell (MSVC or winget path):**

```powershell
./scripts/bootstrap.ps1
```

The script checks for CMake, Ninja, a C compiler, and optional tools. On
success it also runs `cmake --preset ninja-debug`, generating
`build/ninja-debug/compile_commands.json` so editors can find project include
paths.

If something is missing, the script prints a specific error, for example:

```
ERROR: missing ninja. Install Ninja or choose a generator-neutral preset
through CMakeUserPresets.json.
```

For a quick check that skips the configure step:

```sh
./scripts/bootstrap.sh --check-only
```

```powershell
./scripts/bootstrap.ps1 -CheckOnly
```

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
