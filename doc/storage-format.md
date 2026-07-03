# Storage Format Draft

This draft supports the first implementation pass and can evolve before the MVP
release.

## Library Layout

```text
<library-root>/
  .promptlib/
    version
    index.tsv
    folders.tsv
    categories.tsv
  prompts/
    <folder>/
      <prompt-id>/
        current.txt
        metadata.tsv
        versions/
          0001.txt
          0001.tsv
  archive/
```

## Prompt ID

Prompt IDs are stable, lowercase ASCII identifiers generated from the title plus
a short unique suffix.

Example:

```text
summarize-research-paper-a1b2
```

## Prompt Body

Prompt text is stored as plain UTF-8 text in `current.txt`. The application does
not require a special template language for the MVP.

## Metadata

Metadata is stored as tab-separated `key<TAB>value` rows in `metadata.tsv`.

Required keys:

- `id`
- `title`
- `folder`
- `category`
- `created_at`
- `updated_at`
- `current_version`

Optional repeatable keys:

- `tag`
- `description`

Tags are represented in `.promptlib/index.tsv` as a comma-separated list for
fast filtering. The per-prompt `metadata.tsv` file remains the source of truth
for prompt metadata.

## Versioning

Optimized prompts create new files under `versions/`. Promoting a version copies
that body into `current.txt` and updates `current_version`.

Each prompt may contain:

```text
versions/
  index.tsv
  0002.txt
  0002.tsv
```

`index.tsv` records `version`, `promoted`, `created_at`, and `note`.

## Index

`.promptlib/index.tsv` is a rebuildable cache for list and search commands. The
prompt files remain the source of truth.

## Export And Backup

`export` and `backup` produce directory-based libraries with the same layout as a
normal prompt library. These directories can be restored with `promptlib import`.
