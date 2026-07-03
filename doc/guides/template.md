# Using This Template

Use this checklist when starting a new project from this repository.

## Recommended Flow

1. Run the rename script with your PascalCase project name:

   **Linux/macOS**
   ```sh
   ./scripts/rename-project.sh MyNewLib
   ```

   **Windows PowerShell**
   ```powershell
   .\scripts\rename-project.ps1 MyNewLib
   ```

   The script replaces `PromptEditor`, `prompteditor`, and `PROMPTEDITOR` in all
   source, cmake, CI, and documentation files, then renames the include and
   source directories and cmake config files to match.

2. Run `./scripts/bootstrap.sh` (or `.ps1`) to verify the renamed project
   configures and builds.

3. Replace the example API, implementation, executable, and tests.
4. Decide whether the project should support install, package config,
   `add_subdirectory()`, shared libraries, coverage, and AI-agent docs.
5. Trim optional pieces that are out of scope.
6. Run the validation commands in this document and `cmake.md`.

### Manual Rename (Fallback)

If you prefer to rename by hand, update these names together:

- `project(PromptEditor ...)` in `CMakeLists.txt`.
- Public include namespace under `include/prompteditor/`.
- Implementation namespace under `src/prompteditor/`.
- CMake target names such as `prompteditor_core`.
- Package config names in `cmake/Packaging.cmake` and
  `cmake/PromptEditorConfig.cmake.in`.
- Test target names in `cmake/Tests.cmake`.
- README title, CI workflow names, and documentation examples.

## Replace The Example

- Replace `include/prompteditor/example.h` with the first real public API.
- Replace `src/prompteditor/example.c` with the owning implementation.
- Replace `src/main.c` if the project does not ship a CLI or example program.
- Keep tests focused on public behavior in `tests/`.

## Keep Generated Metadata

Version metadata is generated from `project(... VERSION ...)` into
`prompteditor/version.h`. Keep runtime version reporting wired to that generated
header instead of duplicating literal version numbers in source files.

## Validate The Renamed Project

Run the local checks after renaming:

**Linux/macOS**

```sh
./scripts/bootstrap.sh
./scripts/check.sh
cmake --preset ninja-shared
cmake --build --preset ninja-shared
ctest --preset ninja-shared --output-on-failure
```

**Windows PowerShell**

```powershell
./scripts/bootstrap.ps1
./scripts/check.ps1
cmake --preset ninja-shared
cmake --build --preset ninja-shared
ctest --preset ninja-shared --output-on-failure
```

If the project remains installable, also run a package smoke test using
`tests/package_smoke/` as the downstream consumer pattern.

If the project remains consumable through `add_subdirectory()`, keep and run
`tests/subproject_smoke/`.

## Trim Optional Pieces

Keep the template small after renaming. Remove pieces that do not match the new
project:

- Remove `src/main.c` and `PROMPTEDITOR_BUILD_EXAMPLE` if the project has no CLI or
  runnable example.
- Remove `tests/package_smoke/` and package install docs if the project is not
  intended to be installed or consumed through CMake package config.
- Remove `tests/subproject_smoke/` if the project will never be consumed with
  `add_subdirectory()`.
- Remove the `coverage` preset and CI job if coverage data will not be used.
- Remove AI-agent docs if the downstream project does not accept AI-assisted
  changes.
- Keep `.gitattributes`, formatting rules, and local bootstrap/check scripts
  unless the downstream project has a stronger existing policy.

## Documentation To Update

After renaming, update:

- `README.md` for the new project overview and quick start.
- `doc/guides/cmake.md` for project-specific targets, options, and smoke tests.
- `doc/guides/testing.md` for test layers and required checks.
- `doc/guides/release.md` if the project publishes artifacts.
- `doc/guides/ai-agent.md` if the project accepts AI-assisted changes.
- `SECURITY.md` if support windows or vulnerability reporting differ.
