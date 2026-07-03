# AI Agent Policy

AI agents may help with development when their changes are scoped, reviewed,
and verified like any other contribution.

## Required Reading

Before editing, agents should read:

- `AGENTS.md`
- `CONTRIBUTING.md`
- `doc/guides/testing.md`
- `doc/guides/cmake.md` for build, package, or CMake changes.
- This document

## Appropriate Uses

- Focused bug fixes.
- Test additions.
- Documentation updates.
- Build and CI maintenance.
- Mechanical refactors that preserve behavior.

## Restricted Uses

AI agents should not independently:

- Redesign core architecture without maintainer direction.
- Weaken tests, warnings, static analysis, or CI.
- Add large dependencies or generated code without approval.
- Publish releases, rotate credentials, or change repository permissions.

## Disclosure

PRs should disclose meaningful AI assistance, list docs read, summarize what
was changed, and record validation.

## Playbooks

### General Workflow

1. Read `AGENTS.md` and relevant docs.
2. Inspect the smallest relevant file set.
3. Make focused changes.
4. Run relevant checks.
5. Summarize changed files, validation, and residual risk.

For CMake, package, install, or subproject work, use `doc/guides/cmake.md` to
select the relevant smoke checks.

### Bug Fix

1. Reproduce or identify the failing behavior.
2. Add a regression test when practical.
3. Fix the smallest owning module.
4. Run targeted and relevant broader checks.

### Refactor

1. Define behavior that must remain unchanged.
2. Keep changes within module boundaries.
3. Avoid mixing feature behavior with refactor work.
4. Run tests covering touched modules.

### CI Or Build Fix

1. Inspect the failing check and logs.
2. Reproduce locally when practical.
3. Fix the project cause rather than masking the check.
4. Keep scripts and presets aligned.
