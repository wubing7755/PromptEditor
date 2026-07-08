# Contributing

This project uses focused, reviewable changes.

## Local Checks

See [doc/guides/cmake.md](doc/guides/cmake.md) for a detailed explanation of configure, build, test,
install, presets, and common CMake troubleshooting.

Setup and run local checks (`.ps1` on Windows PowerShell):

```sh
./scripts/bootstrap.sh   # first-time setup and configure
./scripts/check.sh       # configure, build, and test
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
- Use `pp` in CLI examples and documentation.

Use the issue templates for bug reports, feature proposals, and infrastructure
maintenance requests.

## Architecture

- Keep public headers small and stable.
- Keep implementation details out of `include/`.
- Prefer explicit ownership and cleanup paths.
- Avoid adding abstractions until repeated use justifies them.

## Comment Style

This section defines the project's comment conventions. The goal is
consistent, high-signal comments that explain **why** rather than
restating **what** the code does.

### Public headers (`include/`)

Use Doxygen-style block comments (`/** ... */`) for every public function,
struct, and macro.  Include `@param` and `@return` tags so that IDEs and
tools can surface structured documentation on hover.

```c
/**
 * Adds two integers and writes the result to out_value.
 *
 * @param left      First operand.
 * @param right     Second operand.
 * @param out_value Pointer to result storage; must not be NULL.
 * @return 1 on success, 0 on overflow or NULL out_value.
 */
PP_API int pp_add_checked(int left, int right, int *out_value);
```

For simple accessors a single-line `/** ... */` is acceptable:

```c
/** Returns the library version compiled into this build. */
PP_API PP_Version pp_version(void);
```

### Implementation files (`src/`)

Use `//` line comments throughout `.c` files.  `//` is part of C11 (the
project's standard) and produces less visual noise than `/* */` for
everyday editing.

**Section dividers** — mark logical groups of related functions:

```c
// ============================================================================
// pp init — initialize a prompt library root
// ============================================================================
```

**Function-level comments** — placed immediately before each function
definition (public entry-points first, then internal helpers):

```c
// Resolves the library root from --root flag, PROMPTLIB_ROOT env var, or
// default path.  Writes result to `out` and returns 1 on success.
static int resolve_root_arg(int start_index, int argc, char **argv,
                            char *out, size_t out_size) {
```

**Inline comments** — use sparingly for non-obvious logic:

```c
if (right > 0 && left > INT_MAX - right) {  // overflow check
    return 0;
}
```
