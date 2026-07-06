# 0001 C11 CMake Toolchain Policy

## Context

This project should work across Windows, Linux, and macOS without baking one
developer's IDE, compiler installation path, or generator choice into shared
project files. Editors and language servers also need a reliable way to discover
the same include directories and language mode used by the build.

## Decision

The project uses CMake as the source of truth for build metadata and C11 as the
portable language baseline. Library targets declare that requirement with
`target_compile_features(... c_std_11)`.

The supported compiler families are MSVC, GCC, Clang, and AppleClang. Shared
presets include generator-neutral names so developers can choose Visual Studio,
Xcode, Ninja, Make, or another CMake-supported generator appropriate for their
platform.

The primary editor and CI workflow uses explicit `ninja-*` presets. Those
presets are generator-dependent by design, but remain portable because they do
not include local compiler paths, SDK paths, or IDE kit selections. They provide
a stable `compile_commands.json` location for clangd and clang-tidy.

Machine-specific compiler paths, SDK paths, and IDE kit selections belong in
local user configuration, not in committed project files.

Portable editor template files may be committed when they do not select one
developer's machine. Recommended VS Code extensions, CMake preset tasks, generic
include fallbacks, and clangd fallback flags are allowed. Absolute compiler
paths and SDK installation paths are local configuration.

## Consequences

Consumers that link the exported CMake target inherit the C11 requirement.
The `ninja-*` presets provide `compile_commands.json` for language servers.
Multi-config IDE generators remain supported through the generator-neutral
presets, but editor integration should use CMake metadata or local workspace
configuration when a language server cannot consume the generator directly.
