# Architecture Decision Records

Use ADRs for stable decisions that affect future contributors.

Create an ADR when a decision changes project structure, public API direction,
dependency policy, packaging strategy, or long-term testing and CI expectations.
Do not create ADRs for routine implementation details or one-off bug fixes.

Each ADR should include:

- Context
- Decision
- Consequences

Keep ADRs short. Detailed design notes belong in source comments only when
they explain local complexity.

Use filenames like:

```text
0001-use-cmake-presets.md
0002-support-shared-library-builds.md
```

## Records

- [0001 C11 CMake Toolchain Policy](0001-c11-cmake-toolchain-policy.md)
