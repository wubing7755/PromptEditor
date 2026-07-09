# AGENTS.md

Repository-specific instructions for AI agents.

## Required Reading

Before changing files, read:

- `CONTRIBUTING.md`
- `docs/guides/testing.md`

For architecture-sensitive changes, also read:

- `docs/adr/README.md`
- Relevant ADR files under `docs/adr/`

For build, CI, packaging, release, dependency, or security work, also read:

- `docs/guides/cmake.md`
- `docs/guides/release.md`
- `SECURITY.md`

## Project Shape

This is a C11 project using CMake and CTest.

```text
include/        Public headers
src/            Implementation
tests/          CTest tests
cmake/          Build modules
scripts/        Local bootstrap and check wrappers
docs/            Contributor documentation
docs/guides/     Topic guides
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

## AI Agent Policy

### Appropriate Uses

- Focused bug fixes.
- Test additions.
- Documentation updates.
- Build and CI maintenance.
- Mechanical refactors that preserve behavior.

### Restricted Uses

AI agents should not independently:

- Redesign core architecture without maintainer direction.
- Weaken tests, warnings, static analysis, or CI.
- Add large dependencies or generated code without approval.
- Publish releases, rotate credentials, or change repository permissions.

### Disclosure

PRs should disclose meaningful AI assistance: list docs read, summarize changes,
and record validation performed.

### Playbooks

**General workflow:** Read AGENTS.md and relevant docs, inspect minimal file set,
make focused changes, run checks, summarize.

**Bug fix:** Reproduce, add regression test, fix smallest owning module, run checks.

**Refactor:** Define preserved behavior, stay within module boundaries, avoid
mixing features with refactors, run tests covering touched modules.

**CI / build fix:** Inspect failing check, reproduce locally, fix the project cause
rather than masking the check, keep scripts and presets aligned.

## Documentation

Update local docs when build commands, CMake usage, dependencies, testing policy,
release policy, security policy, or public API expectations change.
