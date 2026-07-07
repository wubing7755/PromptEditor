# Prompt Library C App MVP

Prompt Library is a pure C command-line application for saving, organizing,
retrieving, and improving prompt templates in a local file-backed library.

## Goals

- Store prompts in a local folder users control.
- Organize prompts by folder, category, and tags.
- Retrieve prompts quickly by id, title, folder, category, tag, or text search.
- Preserve prompt history when creating optimized versions.
- Keep prompt files human-readable and portable.
- Make the CLI useful in scripts by supporting raw prompt output.

## Non-Goals For The MVP

- No network sync.
- No hosted service or database server.
- No graphical interface.
- No model-provider integration for automatic optimization.
- No multi-user locking beyond safe local file writes.

## Command Surface

### `pp init`

Initializes a prompt library root.

Planned options:

- `--root <path>`: create or validate a specific library root.

If `--root` is omitted, `PROMPTLIB_ROOT` is used. If that is unset, the command
uses a default `.promptlib` folder in the current user's home directory.

### `pp add`

Saves a new prompt.

Implemented options:

- `--title <text>`: required prompt title.
- `--body <text>`: prompt body inline.
- `--folder <path>`: logical folder inside the library.
- `--category <name>`: category name.
- `--description <text>`: optional summary.
- `--root <path>`: library root.
- `--tag <name>`: repeatable tag.

Planned later:

- `--editor`: Open $EDITOR to enter prompt body.
- `--file <path>`: read prompt body from a file.

### `pp list`

Lists prompts.

Implemented options:

- `--root <path>`: library root.
- `--folder <path>`: filter by folder.
- `--category <name>`: filter by category.
- `--tag <name>`: filter by tag.
- `--json`: output in JSON format.
- `--no-pager`: disable automatic paging.

Planned later:

- `--format table|plain`: choose output shape.

### `pp folder <list|create|remove|rename>`

Manages folder names in the library.

Implemented options:

- `--root <path>`: library root.
- `--to <name>`: target name for `rename`.
- `--yes`: confirm remove.

Folder remove is blocked while prompts still use the folder.

### `pp category <list|create|remove|rename>`

Manages category names in the library.

Implemented options:

- `--root <path>`: library root.
- `--to <name>`: target name for `rename`.
- `--yes`: confirm remove.

Category remove is blocked while prompts still use the category.

### `pp show <id-or-title>`

Displays a prompt.

Implemented options:

- `--root <path>`: library root.
- `--raw`: print only the prompt body.
- `--json`: output in JSON format.

Planned later:

- `--metadata`: print metadata without body.

### `pp edit <id-or-title>`

Updates prompt metadata or content.

Implemented options:

- `--body <text>`
- `--category <name>`
- `--description <text>`
- `--root <path>`
- `--tag <name>`: replace tags; repeat for multiple tags.
- `--title <text>`
- `--folder <path>`

Planned later:

- `--file <path>`
- `--remove-tag <name>`

### `pp delete <id-or-title>`

Archives or removes a prompt.

Implemented options:

- `--yes`: confirm destructive behavior.

Prompts are archived by default.

Planned later:

- `--permanent`: permanently remove the prompt file.

### `pp search <query>`

Searches title, body, description, category, folder, and tags.

Implemented options:

- `--folder <path>`
- `--category <name>`
- `--tag <name>`
- `--raw`: print raw bodies for matching prompts.
- `--json`: output in JSON format.
- `--root <path>`: library root.

Planned later:

- Search prompt descriptions after metadata parsing is centralized.

### `pp optimize <id-or-title>`

Creates an improved prompt version without overwriting the original.

Implemented options:

- `--body <text>`: optimized prompt body.
- `--note <text>`: rationale or change note.
- `--promote`: make the optimized version current.
- `--history`: list optimized versions.
- `--compare <version>`: compare current body with a version.
- `--root <path>`: library root.

Planned later:

- `--file <path>`: read optimized body from a file.

### `pp export`

Exports prompts or a subtree.

Implemented options:

- `--folder <path>`
- `--out <path>`
- `--root <path>`

Planned later:

- `--category <name>`
- compressed archive output.

### `pp import <path>`

Imports prompts from another library folder or export.

Implemented options:

- `--on-conflict skip|rename|replace`: conflict strategy.
- `--root <path>`

Current conflict strategies:

- `skip`: default, leaves existing prompts unchanged.
- `replace`: updates the index and copies imported prompt files.

### `pp backup`

Creates a restorable backup of the active library.

Implemented options:

- `--out <path>`
- `--root <path>`

## Environment

- `PROMPTLIB_ROOT` points to the active library root.
- If unset, the application uses a platform-appropriate default under the current user's home directory.

## Validation

- Folder paths cannot be absolute paths and cannot contain `..`.
- Metadata fields reject tabs and newlines.
- Tags reject commas because comma is used for the index representation.
- Destructive folder/category and prompt delete operations require explicit confirmation.

## First Implementation Path

1. Implement `init` and library root discovery.
2. Implement `add`, `list`, and `show`.
3. Add metadata filters.
4. Add `edit` and safe `delete`.
5. Add `search`.
6. Add versioned `optimize`.
7. Add `export`, `import`, and `backup`.



