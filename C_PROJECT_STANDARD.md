# C Project Standard

This document defines a pragmatic baseline for small and medium C projects.

## Core Principles

- Start simple and keep seams visible.
- Put public APIs in `include/`; keep implementation details in `src/`.
- Add abstraction only after repeated concrete use.
- Make build, test, and CI reliable from the first commit.
- Keep documentation short, local, and tied to decisions that affect contributors.

## Required Project Shape

```text
include/<project>/   Public headers
src/<project>/       Library implementation
src/main.c           Optional CLI or app entry point
tests/               Focused CTest tests
cmake/               CMake modules
scripts/             Local bootstrap and check wrappers
doc/                 Short documentation
doc/guides/          Topic guides
.github/workflows/   CI
```

## CMake Standard

Use a small top-level `CMakeLists.txt` that delegates to modules:

- `cmake/CompilerOptions.cmake` for warnings, sanitizers, and coverage.
- `cmake/Dependencies.cmake` for third-party dependencies.
- `cmake/Sources.cmake` for source lists.
- `cmake/Tests.cmake` for test targets.
- `cmake/Packaging.cmake` for install and packaging rules.

Installable libraries should generate CMake package config files so downstream
projects can use `find_package(<Project> CONFIG REQUIRED)`.

Libraries should support `BUILD_SHARED_LIBS` unless the project has a reason to
ship only static or only shared artifacts. Public headers should expose a small
export macro and be usable from C++ callers with `extern "C"` guards.

Top-level-only conveniences such as examples, tests, and install rules should
default to on for normal repository builds and off when the project is included
as a subproject.

Projects that support both installation and subproject consumption should keep
small smoke tests for both paths.

Language and toolchain policy should be explicit:

- Set the project language standard in CMake as a target-level requirement, such
  as `target_compile_features(<library> PUBLIC c_std_11)`.
- Use C11 as the default baseline for portable C libraries unless the project
  documents a reason to require a newer standard.
- Support compiler families rather than one exact local compiler path: MSVC,
  GCC, Clang, and AppleClang are the normal baseline.
- Keep committed presets generator-neutral unless a preset exists specifically
  to document a generator-dependent workflow.
- Prefer explicit `ninja-*` presets for the default editor and CI workflow when
  the project wants a stable `compile_commands.json` for clangd.
- Keep compiler installation paths, SDK paths, and developer-specific CMake kits
  in user presets or local IDE configuration, not in shared project files.
- Provide a `CMakeUserPresets.example.json` when the repository supports common
  local generator alternatives.
- Provide a bootstrap script when first-run failures are likely to be toolchain
  or environment issues rather than source issues.
- Document which tools users must install and which tools are optional editor
  conveniences. A repository should not assume CMake, a compiler, Ninja, VS Code
  extensions, clangd, or static-analysis tools are already present.
- Commit only portable editor template files. VS Code recommendations, generic
  CMake tasks, `.clangd` fallback flags, and optional fallback examples are
  useful; absolute compiler paths and personal workspace settings are not.

Recommended primary presets:

- `ninja-debug`
- `ninja-release`
- `ninja-shared`
- `ninja-asan`
- `ninja-coverage`

Recommended generator-neutral compatibility presets:

- `debug`
- `release`
- `shared`
- `asan`
- `coverage`

## Testing Standard

- Use CTest as the universal test runner.
- Prefer focused test executables per module or subsystem.
- Keep test helpers in `tests/support/`.
- Do not test implementation details unless the detail is a documented internal contract.
- Bug fixes should include a regression test unless the behavior is only observable manually.
- Installable libraries should include a package smoke test that builds a small
  downstream project with `find_package`.

## Header Boundary Standard

Public:

```text
include/<project>/feature.h
```

Internal:

```text
src/<project>/feature_internal.h
```

Rules:

- Public headers should include only what they need.
- Public types should be stable and documented.
- Internal headers may expose implementation state but must stay out of public consumers.

## CI Standard

Every project should have:

- Debug build and tests across supported compiler families (MSVC, GCC, Clang,
  AppleClang).
- Release build and tests.
- Shared-library build and tests when `BUILD_SHARED_LIBS` is supported.
- Sanitizer build where supported, across all platforms and compilers that support
  the sanitizer flags.
- Static analysis using at least `cppcheck`.
- clang-tidy when a project provides a `.clang-tidy` configuration.
- YAML linting for workflow and automation files.
- Coverage preset validation for projects that provide a coverage preset.
- Package smoke test for installable libraries across platforms.

## Repository Maintenance Standard

Templates should include lightweight PR and issue templates when the repository
expects external or AI-assisted contributions. GitHub Actions dependencies
should be covered by Dependabot or an equivalent documented update process.

## Documentation Standard

Keep local docs short:

- `README.md` for overview and quick start.
- `CONTRIBUTING.md` for workflow and PR expectations.
- `SECURITY.md` for vulnerability handling.
- `doc/README.md` for the documentation map and source-of-truth index.
- `doc/guides/environment.md` for tool installation and environment setup.
- `doc/guides/cmake.md` for CMake workflows, presets, package consumption, and
  troubleshooting.
- `doc/guides/testing.md` for test layers and expectations.
- `doc/guides/release.md` for release steps.
- `doc/guides/template.md` for rename and trim guidance.
- `doc/guides/ai-agent.md` for AI-assisted contribution rules.
- `doc/adr/README.md` for architecture decisions.
- `AGENTS.md` for AI-agent instructions.

## Avoid By Default

Do not add these early unless the project already needs them:

- Plugin registries.
- Descriptor manifests.
- Multi-stage command dispatch systems.
- Global service locator objects.
- Generated code.
- Large vendored dependencies.

Use the smallest design that keeps ownership clear.
