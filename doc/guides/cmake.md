# CMake Guide

This guide explains the common CMake workflows in this project. Use it when
you are new to CMake or when you need to add source files, tests, install rules,
or downstream consumers.

## Requirements

These tools are not provided by the repository. Install them before configuring
the project.

- CMake 3.21 or newer.
- A C11 compiler.
- One supported compiler family:
  - MSVC on Windows.
  - GCC or Clang on Linux.
  - AppleClang or Clang on macOS.
- Ninja for the default presets, or another generator supported by CMake through
  local presets or command-line generator selection.

For step-by-step installation commands per platform, see
[environment.md](environment.md).

The project requires C11 at the target level with `c_std_11`. Keep public
headers portable across the supported compiler families and avoid compiler
extensions unless they are isolated behind small CMake/compiler checks.

## Tool Installation Notes

Typical setup choices:

| Platform | Compiler | Generator |
| --- | --- | --- |
| Windows (MSVC) | MSVC from Visual Studio Build Tools or Visual Studio | Ninja |
| Windows (MSYS2) | Clang or GCC from MSYS2 UCRT64 | Ninja |
| Linux | GCC or Clang from the system package manager | Ninja |
| macOS | AppleClang from Xcode Command Line Tools or Clang | Ninja |

On Windows, use the Ninja presets from a Developer PowerShell, Developer
Command Prompt, or a VS Code CMake kit that initializes MSVC. A normal shell may
find CMake but still fail to find `cl.exe` when Ninja is selected.

VS Code is optional. If you use it, install CMake Tools plus clangd. The default
workspace settings use the `ninja-debug` preset, and `.clangd` points clangd at
`build/ninja-debug/compile_commands.json`. Microsoft C/C++ remains usable as a
local fallback when paired with CMake Tools, but do not enable multiple C/C++
language services for the same workspace unless you intentionally manage their
overlap.

The project commits portable editor defaults:

- `.vscode/extensions.json` recommends CMake Tools and clangd.
- `.vscode/settings.json` tells VS Code to use the `ninja-debug` CMake preset,
  and disables Microsoft C/C++ IntelliSense so clangd is the active language
  service.
- `.vscode/c_cpp_properties.example.json` provides optional Microsoft C/C++
  fallback include paths without hard-coded compiler installation paths.
- `.vscode/tasks.json` provides configure, build, and test tasks that call CMake
  and CTest directly.
- `.clangd` points clangd at the Ninja compile database and provides fallback
  project include paths.

Keep these files generic. Do not commit absolute compiler paths such as a local
Visual Studio, MSYS2, Homebrew, or system SDK installation. Put those paths in
VS Code user settings, CMake kits, `CMakeUserPresets.json`, shell environment
setup, or a local toolchain file.

## Mental Model

CMake work usually has four separate steps:

1. Configure: read `CMakeLists.txt`, choose options, and generate build files.
2. Build: compile sources and link targets using the generated build files.
3. Test: run CTest tests from the configured build tree.
4. Install: copy public artifacts into an install prefix for downstream use.

Those steps are intentionally separate. If configure fails, the build files are
not reliable. If build fails, tests and install are not meaningful.

## Quick Commands

```sh
./scripts/bootstrap.sh   # or .\scripts\bootstrap.ps1 on Windows
./scripts/check.sh       # or .\scripts\check.ps1 on Windows
```

The check script runs configure, build, and tests. For one step at a time:

```sh
cmake --preset ninja-debug
cmake --build --preset ninja-debug
ctest --preset ninja-debug --output-on-failure
```

## Presets

Presets are named build configurations stored in `CMakePresets.json`.

| Preset | Purpose |
| --- | --- |
| `ninja-debug` | Default cross-platform development build with debug symbols and `compile_commands.json`. |
| `ninja-release` | Optimized release-style validation and install smoke tests with Ninja. |
| `ninja-shared` | `ninja-debug` build with `BUILD_SHARED_LIBS=ON`. |
| `ninja-asan` | Ninja debug build with AddressSanitizer and UBSan where supported. |
| `ninja-coverage` | Ninja debug build with coverage flags where supported. |
| `debug` | Fast local development with debug symbols. |
| `release` | Optimized build for release-style validation and install smoke tests. |
| `shared` | `debug` build with `BUILD_SHARED_LIBS=ON`. |
| `asan` | Debug build with AddressSanitizer and UBSan where supported. |
| `coverage` | Debug build with coverage flags where supported. |

Configure, build, and test presets share the same names:

```sh
cmake --preset ninja-shared
cmake --build --preset ninja-shared
ctest --preset ninja-shared --output-on-failure
```

The `ninja-*` presets are the default editor and CI path. They force the Ninja
generator so CMake writes `compile_commands.json` into the preset build
directory. Keep Ninja installed on every development platform that uses these
defaults.

The `debug`, `release`, `shared`, `asan`, and `coverage` presets remain
generator-neutral compatibility entries. Use them from a `CMakeUserPresets.json`
file, a CMake kit, or explicit command-line generator selection when you need
Visual Studio, Xcode, Make, or another generator:

```sh
cmake --preset debug -G "<your generator>"
```

To create local presets, copy `CMakeUserPresets.example.json` to
`CMakeUserPresets.json` and edit the generator names for your machine. The real
`CMakeUserPresets.json` file is ignored by Git.
For Visual Studio workflows, add a local preset that inherits from `debug` and
sets your installed generator, such as `Visual Studio 17 2022` or a newer local
version.

Visual Studio and Xcode builds remain supported, but they are multi-config
generators. Use CMake Tools as the configuration provider for IDE IntelliSense
or add editor-local include paths for generated headers when the language server
cannot consume the generator metadata directly.

All project presets inherit a base preset that enables
`CMAKE_EXPORT_COMPILE_COMMANDS`. Generators that support compilation databases
write `compile_commands.json` into the preset build directory.

## Important Options

These options can be passed during configure:

```sh
cmake --preset ninja-debug -DPP_BUILD_EXAMPLE=OFF
```

| Option | Meaning |
| --- | --- |
| `BUILD_SHARED_LIBS` | Builds libraries as shared libraries when supported. |
| `PP_BUILD_EXAMPLE` | Builds `src/main.c` as the example executable. |
| `PP_BUILD_TESTING` | Builds CTest test targets. |
| `PP_INSTALL` | Generates install rules. |
| `PP_ENABLE_ASAN` | Enables AddressSanitizer where supported. |
| `PP_ENABLE_UBSAN` | Enables UndefinedBehaviorSanitizer where supported. |
| `PP_ENABLE_COVERAGE` | Enables coverage flags where supported. |

Examples, tests, and install rules default to on for top-level builds. They
default to off when this project is included through `add_subdirectory()`.

Use `cmake -LAH -S . -B build/ninja-debug` after configuring if you need to
inspect available cache options.

## Multi-Config Generators

Visual Studio and Xcode are multi-config generators. They can produce Debug and
Release artifacts from the same build directory, so build and test commands need
an explicit config in some workflows:

```sh
cmake --build build/package-smoke --config Release
ctest --test-dir build/package-smoke -C Release --output-on-failure
```

Single-config generators such as Ninja and Make use `CMAKE_BUILD_TYPE` at
configure time instead.

## Toolchain Policy

CMake 3.21+, C11 (`c_std_11`), Ninja default generator. Supported compilers:
MSVC, GCC, Clang, AppleClang. See [ADR 0001](../adr/0001-c11-cmake-toolchain-policy.md)
for the full policy.

## Add A Source File

Add implementation files under `src/prompteditor/` and public headers under
`include/prompteditor/`.

1. Create the `.c` file under `src/prompteditor/`.
2. Create or update the matching public header under `include/prompteditor/`.
3. Add the new `.c` file to `PP_CORE_SOURCES` in `cmake/Sources.cmake`.
4. Add tests under `tests/`.
5. Run `./scripts/check.ps1` or `./scripts/check.sh`.

Public headers should include only what they need and should remain usable from
C++ callers when they expose C APIs.

## Add A Test

Use `pp_add_test()` from `cmake/Tests.cmake`:

```cmake
pp_add_test(pp_feature_tests
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_feature.c
    LIBS
        prompteditor_core
)
```

Then run:

```sh
cmake --preset ninja-debug
cmake --build --preset ninja-debug
ctest --preset ninja-debug --output-on-failure
```

Tests should assert public behavior. Keep implementation-detail tests rare and
document the internal contract they protect.

## Install And Use With find_package

```sh
cmake --preset ninja-release
cmake --build --preset ninja-release
cmake --install build/ninja-release --config Release --prefix install
```

A downstream CMake project can then consume it:

```cmake
find_package(PromptEditor CONFIG REQUIRED)
target_link_libraries(app PRIVATE PromptEditor::prompteditor_core)
```

Validate with the package smoke test (`.ps1` on Windows):

```sh
cmake -G Ninja -S tests/package_smoke -B build/package-smoke -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$PWD/install"
cmake --build build/package-smoke --config Release
ctest --test-dir build/package-smoke -C Release --output-on-failure
```

For shared-library installs, use the `ninja-shared` preset instead. On Windows,
copy the installed DLL next to the test executable or add it to `PATH`.

## Use As A Subproject

```cmake
add_subdirectory(path/to/PromptEditor)
target_link_libraries(app PRIVATE PromptEditor::prompteditor_core)
```

When included as a subproject, examples, tests, and install rules default to off.
Validate with the subproject smoke test:

```sh
cmake -G Ninja -S tests/subproject_smoke -B build/subproject-smoke -DPP_SOURCE_DIR="$PWD"
cmake --build build/subproject-smoke
ctest --test-dir build/subproject-smoke --output-on-failure
```

## Generated Version Header

The project version is declared once:

```cmake
project(PromptEditor VERSION 0.1.0 LANGUAGES C)
```

CMake generates `prompteditor/version.h` from that value. Source files should use the
generated macros instead of duplicating literal version numbers.

## Local Quality Checks

Run bootstrap after cloning, after installing tools, or after switching to a new
machine:

**Windows PowerShell**

```powershell
./scripts/bootstrap.ps1
```

**Linux/macOS**

```sh
./scripts/bootstrap.sh
```

The check scripts run configure, build, and tests. They also run optional tools
when available:

**Windows PowerShell**

```powershell
./scripts/check.ps1 -EnableTidy
```

**Linux/macOS**

```sh
./scripts/check.sh --enable-tidy
```

If `clang-format`, `clang-tidy`, or `yamllint` is not installed locally, use CI
as the authoritative check for those tools.

## Troubleshooting

Old CMake cache points at another path:

- Symptom: configure mentions a different source directory.
- Fix: delete the affected build directory, such as `build/ninja-release`, and
  run `cmake --preset ninja-release` again.

Ninja or the C compiler is missing:

- Symptom: configure says it cannot find the Ninja build program or
  `CMAKE_C_COMPILER` is not set.
- Fix: run `./scripts/bootstrap.ps1` or `./scripts/bootstrap.sh` for a clearer
  diagnostic. On Windows, open Developer PowerShell, Developer Command Prompt,
  or select an MSVC CMake kit in VS Code before configuring a Ninja preset.

VS Code builds `ALL_BUILD` with Ninja:

- Symptom: CMake Tools runs `cmake --build ... --target ALL_BUILD` and Ninja
  reports `unknown target 'ALL_BUILD'`.
- Fix: run `CMake: Set Build Target` and select `all`. If the old target is
  still cached, run `CMake: Reset CMake Tools Extension State`, then
  `CMake: Delete Cache and Reconfigure`.

`find_package(PromptEditor)` cannot find the package:

- Symptom: CMake cannot find `PromptEditorConfig.cmake`.
- Fix: install the project first and pass the absolute install prefix with
  `-DCMAKE_PREFIX_PATH=/path/to/install`.

Windows shared-library test cannot find the DLL:

- Symptom: an installed shared-library consumer builds but fails at runtime.
- Fix: copy the installed DLL next to the executable or add the install `bin`
  directory to `PATH`.

`ctest` cannot find a test executable with Visual Studio:

- Symptom: CTest looks in the wrong configuration directory.
- Fix: pass `-C Debug` or `-C Release`, matching the build config.

`clang-format`, `clang-tidy`, or `yamllint` is missing:

- Symptom: local scripts skip a tool or the shell cannot find it.
- Fix: install the tool locally, or rely on CI where the workflow installs the
  required toolchain.
