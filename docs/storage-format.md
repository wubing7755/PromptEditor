# Storage Format

This document specifies the PromptLib library file layout and data formats.

## Library Layout

```text
<library-root>/
  .promptlib/
    version           → "1"
    index.tsv         → rebuildable prompt cache
    folders.tsv       → registered folder names
    categories.tsv    → registered category names
  prompts/
    <folder>/
      <prompt-id>/
        current.md    → active prompt body (Markdown)
        metadata.tsv  → per-prompt metadata (source of truth)
        versions/
          index.tsv   → version manifest
          NNNN.md     → snapshot body (0002–9999)
          NNNN.tsv    → snapshot metadata
  archive/
    <prompt-id>/      → archived prompt (identical layout to active prompt)
      current.md
      metadata.tsv
      versions/
```

## Prompt ID

Prompt IDs are stable, lowercase ASCII identifiers generated from the title plus
a short unique suffix.

Algorithm: slugify title (lowercase alphanumeric, non-alphanumeric → `-`,
trim edges), then append `-` + last 4 hex digits of a djb2 hash of the
original title. Empty slug → `"p"`.

| Input | Output |
|-------|--------|
| `"Hello World! 2024"` | `hello-world-2024-a1b2` |
| `"!!!"` | `p-c3d4` |

IDs are ≤ 128 characters and unique per distinct title.

## Prompt Body

Prompt text is stored as Markdown in `current.md`. The application
does not require a special template language.

## Metadata

Metadata is stored as tab-separated `key<TAB>value` rows in `metadata.tsv`.

### Required Keys

| Key | Description |
|-----|-------------|
| `id` | Stable prompt identifier |
| `title` | Display title |
| `folder` | Container folder name |
| `category` | Functional classification |
| `created_at` | ISO-8601 UTC timestamp |
| `updated_at` | ISO-8601 UTC timestamp |
| `current_version` | Active version number (`1` = implicit v1) |

### Optional Keys

| Key | Description |
|-----|-------------|
| `tag` | Repeatable; one per row |
| `description` | Free-text summary |

### Constraints

- Keys and values must not contain `\t`, `\n`, or `\r`.
- Values must be < 512 characters (`PP_FIELD_MAX`).
- Tag values must not contain commas (comma = list separator in `index.tsv`).

> **Design principle:** `metadata.tsv` in each prompt directory is the
> authoritative source of truth. `index.tsv` is a rebuildable cache.

## Versioning

Version `1` is implicit (the initial body at `current.md`). Optimized
prompts create new numbered snapshots under `versions/`.

```text
versions/
  index.tsv     → columns: version, promoted, created_at, note
  0002.md       → body snapshot
  0002.tsv      → snapshot metadata
```

- Version numbers are 4-digit zero-padded decimals (`0002`–`9999`).
- Auto-increment from the highest existing version.
- Promoting a version copies its body into `current.md` and updates
  `current_version` in `metadata.tsv`.

## Index (Cache)

`.promptlib/index.tsv` is a rebuildable cache for fast list and search commands.
It contains one row per active prompt:

```text
id<TAB>title<TAB>folder<TAB>category<TAB>tags<TAB>updated_at
```

- `tags` is a comma-separated list (no spaces).
- The file begins with a header row: `id\ttitle\tfolder\tcategory\ttags\tupdated_at`.

Run `pp reindex` to rebuild `index.tsv` from individual `metadata.tsv` files.

## Registries

### folders.tsv

```text
name
inbox
work
projects
```

- Header: `name`.
- Default entry: `inbox`.
- Folder names must not contain `..`, `/`, `\`, or `:`.

### categories.tsv

```text
name
general
coding
writing
```

- Header: `name`.
- Default entry: `general`.

## Archive

When a prompt is deleted, its entire directory is moved from
`prompts/<folder>/<prompt-id>/` to `archive/<prompt-id>/`, preserving
`current.md`, `metadata.tsv`, and `versions/` intact. The index row is
removed.

- `pp archive list` — list archived prompts.
- `pp archive restore <id>` — move back to original folder (or `inbox` if
  unavailable) and re-add to index.
- `pp archive purge <id> --yes` — permanently delete from disk.

Duplicate archive IDs are rejected (a prompt cannot be archived twice).

## Export And Backup

`pp export` and `pp backup` produce directory-based libraries with the same
layout as a normal prompt library. These directories can be restored with
`pp import`.

- **Export** can filter by folder; refuses to overwrite existing destinations.
- **Backup** is equivalent to unfiltered export.
- **Import** validates that the source contains `.promptlib/index.tsv` and
  supports `skip` (default) and `replace` conflict strategies.
