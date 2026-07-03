# AGENTS.md

Repository-specific instructions for AI agents.

## Required Reading

Before changing files, read:

- `CONTRIBUTING.md`
- `doc/guides/testing.md`
- `doc/guides/ai-agent.md`

For architecture-sensitive changes, also read:

- `doc/adr/README.md`
- Relevant ADR files under `doc/adr/`

For build, CI, packaging, release, dependency, or security work, also read:

- `doc/guides/cmake.md`
- `doc/guides/release.md`
- `SECURITY.md`

## Project Shape

This is a C11 project template using CMake and CTest.

```text
include/        Public headers
src/            Implementation
tests/          CTest tests
cmake/          Build modules
scripts/        Local bootstrap and check wrappers
doc/            Contributor documentation
doc/guides/     Topic guides
```

## Build And Verification

```sh
cmake --preset ninja-debug
cmake --build --preset ninja-debug
ctest --preset ninja-debug --output-on-failure
```

On Windows PowerShell:

```powershell
./scripts/check.ps1
```

On Linux/macOS:

```sh
./scripts/check.sh
```

## Ownership Rules

- Public APIs belong in `include/`.
- Private implementation helpers belong in `src/`.
- Tests should cover behavior, not accidental implementation details.
- Keep CMake target dependencies narrow.
- Do not add new global state without a documented reason.
- Do not weaken warnings, tests, or CI to make a change pass.

## Documentation

Update local docs when build commands, CMake usage, dependencies, testing policy,
release policy, security policy, or public API expectations change.
