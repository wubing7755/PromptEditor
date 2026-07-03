# Contributing

This project uses focused, reviewable changes.

## Local Checks

See [doc/guides/cmake.md](doc/guides/cmake.md) for a detailed explanation of configure, build, test,
install, presets, and common CMake troubleshooting.

For a new checkout or a new machine, run bootstrap first.
If you have not yet installed the required tools, see
[doc/guides/environment.md](doc/guides/environment.md) for platform-specific
instructions.

**Linux/macOS**

```sh
./scripts/bootstrap.sh
```

**Windows PowerShell**

```powershell
./scripts/bootstrap.ps1
```

**Linux/macOS**

```sh
./scripts/check.sh
```

**Windows PowerShell**

```powershell
./scripts/check.ps1
```

Or run CMake directly:

```sh
cmake --preset ninja-debug
cmake --build --preset ninja-debug
ctest --preset ninja-debug --output-on-failure
```

## Branches

Use short branch names:

- `feature/<topic>`
- `fix/<topic>`
- `refactor/<topic>`
- `infra/<topic>`
- `release/<version>`

## Commits And PRs

Use Conventional Commit style where practical:

```text
fix(parser): reject invalid delimiters
feat(cli): add version command
refactor(build): share test target helper
```

Before opening a PR:

- Run the smallest local checks that cover the change.
- Fill in the PR template sections that apply to the change.
- Explain behavioral, compatibility, or release impact.
- Add tests for behavior changes.
- Link to updated docs when the change affects build, test, install, release,
  security, or public API behavior.
- Keep unrelated cleanup out of feature or fix PRs.

Use the issue templates for bug reports, feature proposals, and infrastructure
maintenance requests.

## Architecture

- Keep public headers small and stable.
- Keep implementation details out of `include/`.
- Prefer explicit ownership and cleanup paths.
- Avoid adding abstractions until repeated use justifies them.
