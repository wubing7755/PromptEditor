# PromptEditor - Pure C Prompt Management CLI

> [中文版](README.zh.md)

Save, organize, search, browse, and optimize prompt templates in a local
file-backed library - all in pure C with zero external dependencies.

## Features

- **Store prompts** in a local folder you control - plain text files, no database
- **Organize** by folder, category, and tags
- **Search** interactively with `pl browse` (uses `fzf` if available) or via `pl search`
- **Edit** prompt body in your `$EDITOR` with `pl edit <id>`
- **Version** prompts with `pl optimize` and compare/promote history
- **Export/Import/Backup** entire libraries
- **JSON output** with `--json` flag for scripting
- **Colored terminal output** with auto-paging (respects `NO_COLOR`)
- **Cross-platform** - Windows (MinGW/MSVC), Linux, macOS

## Quick Start

```powershell
# Windows PowerShell
git clone <repo>
cd PromptEditor
./scripts/bootstrap.ps1
cmake --preset ninja-debug
cmake --build --preset ninja-debug
```

```sh
# Linux/macOS
git clone <repo>
cd PromptEditor
./scripts/bootstrap.sh
cmake --preset ninja-debug
cmake --build --preset ninja-debug
```

Initialize a library and save your first prompt:

```sh
./build/ninja-debug/bin/prompteditor init
./build/ninja-debug/bin/prompteditor add --title "Hello" --body "This is my first prompt."
./build/ninja-debug/bin/prompteditor list
```

> **Tip:** The default library root is `~/.promptlib` (Linux/macOS) or
> `%USERPROFILE%\.promptlib` (Windows). Override with `--root <path>` or the
> `PROMPTLIB_ROOT` environment variable.

## Commands

| Command | Description |
|---------|-------------|
| `help` | Show global help |
| `init` | Initialize a prompt library folder |
| `add` | Save a new prompt (supports `--editor` to open `$EDITOR`) |
| `list` | List prompts (`--json`, `--no-pager`, filter by folder/category/tag) |
| `show` | Print a prompt by ID or title (`--raw`, `--json`) |
| `edit` | Edit prompt metadata or body (opens `$EDITOR` when no flags given) |
| `delete` | Archive or permanently remove a prompt |
| `search` | Search across titles, bodies, tags, and descriptions (`--json`, `--raw`) |
| `browse` | Interactive browser - uses `fzf` for fuzzy search with preview, or numbered menu fallback |
| `optimize` | Create versioned improvements with history and compare |
| `folder` | Manage folders (`list`, `create`, `remove`, `rename`) |
| `category` | Manage categories (`list`, `create`, `remove`, `rename`) |
| `export` | Export prompts or a subtree to another directory |
| `import` | Import prompts from an export or another library |
| `backup` | Create a full library backup |

Run any command with `--help` for detailed usage.

## Examples

```sh
# Initialize a library
promptlib init --root ./my-prompts

# Add a prompt (opens $EDITOR for body input)
promptlib add --title "Summarize" --editor --root ./my-prompts

# Add with inline body
promptlib add --title "Translate" --body "Translate this to Chinese."   --folder translations --category utility --tag language   --root ./my-prompts

# List all prompts (colored table, auto-paged)
promptlib list --root ./my-prompts

# List as JSON
promptlib list --root ./my-prompts --json

# Show a prompt
promptlib show --root ./my-prompts "translate"

# Show raw body (for piping)
promptlib show --root ./my-prompts "translate" --raw

# Edit body in $EDITOR
promptlib edit --root ./my-prompts "translate"

# Edit specific fields
promptlib edit --root ./my-prompts "translate" --title "Renamed" --category advanced

# Search
promptlib search --root ./my-prompts "Chinese" --json

# Interactive browser
promptlib browse --root ./my-prompts

# Optimize with versioning
promptlib optimize --root ./my-prompts "translate"   --body "Translate the provided text into Chinese." --note "Improved clarity"

# View version history
promptlib optimize --root ./my-prompts "translate" --history

# Promote an optimized version
promptlib optimize --root ./my-prompts "translate" --promote

# Export/Import/Backup
promptlib export --root ./my-prompts --out ./backup
promptlib import ./backup --root ./restored

# Delete (archives by default)
promptlib delete --root ./my-prompts "translate" --yes
```

## Build & Test

### Presets

The project uses CMake presets for common workflows. After bootstrap:

| Preset | Description |
|--------|-------------|
| `ninja-debug` | Debug build with Ninja (default) |
| `ninja-release` | Optimized release build |
| `ninja-shared` | Debug build with shared library |
| `ninja-asan` | Debug build with AddressSanitizer + UBSan |
| `ninja-coverage` | Debug build with coverage flags |

### Quick check

```powershell
# Windows PowerShell
./scripts/check.ps1

# With static analysis
./scripts/check.ps1 -EnableTidy
```

```sh
# Linux/macOS
./scripts/check.sh
./scripts/check.sh --enable-tidy
```

### Manual build

```sh
cmake --preset ninja-debug
cmake --build --preset ninja-debug
ctest --preset ninja-debug --output-on-failure
```

### Supported toolchains

- GCC 8+ (primary, tested on MinGW and Linux)
- Clang 7+ (via `-DCMAKE_C_COMPILER=clang`)
- MSVC 2019+ (via Visual Studio generators)

## Project Structure

```
include/prompteditor/    Public headers
  cli.h                  CLI entry point
  compiler.h             Compiler/platform detection
  export.h               DLL export macros
src/                     Implementation
  cli.c                  CLI implementation
  prompteditor/          Core library sources
  main.c                 Executable entry point
tests/                   CTest test programs
  test_example.c         Core library tests
  package_smoke/         Downstream package smoke tests
  subproject_smoke/      add_subdirectory() smoke test
  support/               Test assertion helpers
cmake/                   CMake build modules
scripts/                 Local bootstrap and check wrappers
doc/                     Documentation
  mvp.md                 MVP feature tracking
  storage-format.md      Library storage layout spec
  guides/                Topic guides (build, test, release, etc.)
  adr/                   Architecture Decision Records
```

## Environment

| Variable | Description |
|----------|-------------|
| `PROMPTLIB_ROOT` | Library root path (overrides default `~/.promptlib`) |
| `EDITOR` | Editor for `pl edit` and `pl add --editor` (default: `notepad`/`vi`) |
| `PAGER` | Pager for long output (default: `more`/`less -R`) |
| `NO_COLOR` | Set to any value to disable ANSI colors |

## Storage Format

Prompts are stored as plain files in a human-readable layout:

```
<library-root>/
  .promptlib/               Internal metadata
    index.tsv               Rebuildable prompt index
    folders.tsv             Registered folders
    categories.tsv          Registered categories
  prompts/                  Prompt content
    <folder>/
      <prompt-id>/
        current.txt         Current prompt body
        metadata.tsv        ID, title, folder, category, tags, timestamps
        versions/           Optimized versions
          index.tsv         Version history
          0002.txt          0002.tsv
  archive/                  Deleted prompts
```

See [doc/storage-format.md](doc/storage-format.md) for full details.

## Documentation

- [MVP Feature Tracking](doc/mvp.md) - implemented and planned commands
- [Storage Format](doc/storage-format.md) - library layout and file format
- [Build Guide](doc/guides/cmake.md) - CMake presets, generators, troubleshooting
- [Testing Guide](doc/guides/testing.md) - CTest, static analysis, smoke tests
- [Environment Setup](doc/guides/environment.md) - platform prerequisites
- [Release Guide](doc/guides/release.md) - versioning and packaging
- [Contributing](CONTRIBUTING.md) - how to contribute
- [Security](SECURITY.md) - security policy

[中文版](README.zh.md)

## License

MIT
