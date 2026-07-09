# PromptEditor — Pure C Prompt Management CLI

> [中文版](README.zh.md)

Save, organize, search, browse, and optimize prompt templates in a local
file-backed library — all in pure C with zero external dependencies.

## Features

- **Store prompts** in a local folder you control — plain text files, no database
- **Organize** by folder, category, and tags
- **Search** interactively with `pp browse` (uses `fzf` if available) or via `pp search`
- **Edit** prompt body in your `$EDITOR` with `pp edit <id>`
- **Version** prompts with `pp optimize` and compare/promote history
- **Manage tags** incrementally with `pp tag add/remove/list`
- **Archive** deleted prompts with restore and purge via `pp archive`
- **Rebuild index** from source-of-truth metadata files with `pp reindex`
- **View statistics** for the entire library with `pp stats`
- **Export / Import / Backup** entire libraries
- **JSON output** with `--json` flag for scripting
- **Colored terminal output** with auto-paging (respects `NO_COLOR`)
- **Cross-platform** — Windows (MinGW/MSVC), Linux, macOS

## Quick Start

```sh
# Clone and build (use .ps1 scripts on Windows PowerShell)
git clone https://github.com/wubing7755/PromptEditor
cd PromptEditor
./scripts/bootstrap.sh
cmake --preset ninja-debug
cmake --build --preset ninja-debug
```

Initialize a library and save your first prompt:

```sh
./build/ninja-debug/bin/pp init
./build/ninja-debug/bin/pp add --title "Hello" --body "This is my first prompt."
./build/ninja-debug/bin/pp list
```

> **Tip:** The default library root is `~/.promptlib` (Linux/macOS) or
> `%USERPROFILE%\.promptlib` (Windows). Override with `--root <path>` or the
> `PROMPTLIB_ROOT` environment variable.

## Commands

### Prompt Management

| Command | Description |
|---------|-------------|
| `add` | Save a new prompt (supports `--editor` to open `$EDITOR`) |
| `list` | List prompts (`--json`, `--no-pager`, filter by folder / category / tag) |
| `show` | Print a prompt by ID or title (`--raw`, `--json`) |
| `edit` | Edit prompt metadata or body (opens `$EDITOR` when no flags given) |
| `delete` | Archive or permanently remove a prompt |

### Search & Browse

| Command | Description |
|---------|-------------|
| `search` | Search across titles, bodies, tags, and descriptions (`--json`, `--raw`) |
| `browse` | Interactive browser — uses `fzf` for fuzzy search with preview, or numbered menu fallback |

### Versioning

| Command | Description |
|---------|-------------|
| `optimize` | Create versioned improvements with history and compare |

### Organization

| Command | Description |
|---------|-------------|
| `folder` | Manage folders (`list`, `create`, `remove`, `rename`) |
| `category` | Manage categories (`list`, `create`, `remove`, `rename`) |
| `tag` | Incremental tag management (`add`, `remove`, `list`) |

### Archive

| Command | Description |
|---------|-------------|
| `archive` | Manage archived prompts (`list`, `restore`, `purge`) |

### Transfer

| Command | Description |
|---------|-------------|
| `export` | Export prompts or a subtree to another directory |
| `import` | Import prompts from an export or another library |
| `backup` | Create a full library backup |

### Maintenance

| Command | Description |
|---------|-------------|
| `reindex` | Rebuild `index.tsv` from per-prompt `metadata.tsv` files (`--dry-run` available) |
| `stats` | Display library statistics — prompt count, folders, tags, disk usage (`--json` available) |

### Meta

| Command | Description |
|---------|-------------|
| `help` | Show global help or command-specific help |
| `init` | Initialize a prompt library folder |

Run any command with `--help` for detailed usage.

## Examples

```sh
# Initialize a library and add prompts
pp init --root ./my-prompts
pp add --title "Summarize" --editor --root ./my-prompts
pp add --title "Translate" --body "Translate this to Chinese." --root ./my-prompts

# List and view
pp list --root ./my-prompts
pp show --root ./my-prompts "translate"

# Search, edit, and manage
pp search "Chinese" --root ./my-prompts
pp edit --root ./my-prompts "translate" --title "Renamed"
pp delete --root ./my-prompts "translate" --yes

# Tag management
pp tag add translate-a1b2 urgent
pp tag list translate-a1b2
pp tag remove translate-a1b2 urgent

# Archive management
pp archive list
pp archive restore translate-a1b2

# Maintenance
pp reindex --root ./my-prompts --dry-run
pp stats --root ./my-prompts
```

## Build & Test

```sh
# Run the full check (configure, build, test)
./scripts/check.sh      # or .\scripts\check.ps1 on Windows

# Or step by step
cmake --preset ninja-debug
cmake --build --preset ninja-debug
ctest --preset ninja-debug --output-on-failure
```

Supported toolchains: GCC 8+, Clang 7+, MSVC 2019+. See [docs/guides/cmake.md](docs/guides/cmake.md) for CMake presets and detailed workflows.

## Project Structure

See [AGENTS.md](AGENTS.md) for the project layout and contributor conventions.

## Environment

| Variable | Description |
|----------|-------------|
| `PROMPTLIB_ROOT` | Library root path (overrides default `~/.promptlib`) |
| `EDITOR` | Editor for `pp edit` and `pp add --editor` (default: `notepad` / `vi`) |
| `PAGER` | Pager for long output (default: `more` / `less -R`) |
| `NO_COLOR` | Set to any value to disable ANSI colors |

## Storage Format

Prompts are stored as plain files in a human-readable layout. See [docs/storage-format.md](docs/storage-format.md) for the full specification.

## Documentation

See [docs/README.md](docs/README.md) for the complete documentation index.

## License

MIT
