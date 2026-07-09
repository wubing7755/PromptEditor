# Testing

Use CTest for automated tests.

```sh
ctest --preset ninja-debug --output-on-failure
```

For command details and generator-specific flags, see `cmake.md`.

## Expectations

- Bug fixes should include regression tests.
- New public APIs should include success and failure cases.
- Numeric APIs should cover relevant boundary and overflow cases.
- Tests should assert behavior, not line coverage.
- Prefer focused test programs over broad integration tests for small modules.
- Installable libraries should keep a small downstream package smoke test.

## Static Analysis

CI runs clang-format, clang-tidy, cppcheck, and YAML linting. Local clang-tidy
checks are optional because tool availability varies by platform:

**Linux/macOS**

```sh
./scripts/check.sh --enable-tidy
```

**Windows PowerShell**

```powershell
./scripts/check.ps1 -EnableTidy
```

## What To Run

| Change type | Minimum useful checks |
| --- | --- |
| C source or public header | `./scripts/check.ps1` or `./scripts/check.sh` |
| Shared-library or export macro change | `ninja-shared` preset and shared package smoke |
| Install/package metadata change | static and shared package smoke |
| Subproject behavior change | `tests/subproject_smoke/` |
| CI or YAML change | `git diff --check` and CI static analysis |
| Release-sensitive change | `ninja-release`, `ninja-shared`, `ninja-asan`, and package smoke |

## Package Smoke

`tests/package_smoke/` is a downstream consumer used to verify installed package
metadata. It should stay small and only assert that installed headers, exported
targets, C linkage, and basic library calls work through `find_package`.

Run package smoke tests for both static and shared installs when shared
libraries are supported.

## Subproject Smoke

`tests/subproject_smoke/` verifies that the project can be consumed with
`add_subdirectory()` without enabling top-level examples, tests, or install
rules by default.

## Manual Checks

Document manual checks in PRs when behavior cannot be exercised by CTest.
Manual checks should include the command, platform, and observed result.
