# Release

This project uses automated CI release pipelines triggered by Git tags.

## Contents

- [Release Assets](#release-assets)
- [Automated Release (Recommended)](#automated-release-recommended)
- [Local Release](#local-release)
- [Platform-Native Installers](#platform-native-installers)
- [Release Checklist](#release-checklist)
- [Verification](#verification)


## Release Assets

Each release produces the following platform-specific assets:

| Platform | Binary Archive | Graphical Installer |
|----------|:---:|:---:|
| Windows | `pp-<ver>-windows-x64.zip` | `pp-setup-<ver>.exe` (Inno Setup) |
| Linux | `pp-<ver>-linux.tar.gz` | `pp_<ver>_amd64.deb` (dpkg-deb) |
| macOS | `pp-<ver>-macos.tar.gz` | `pp-<ver>.dmg` (hdiutil) |

## Automated Release (Recommended)

Push a version tag to trigger the CI workflow (`.github/workflows/release.yml`):

```sh
git tag v0.2.0
git push origin v0.2.0
```

The workflow will:

1. Build on all three platforms (Windows/MSVC, Linux/GCC, macOS/AppleClang).
2. Run the full test suite via `ctest` on each platform.
3. Package binaries and platform-native installers.
4. Create a GitHub Release with all assets attached.

The workflow can also be triggered manually via `workflow_dispatch` on the
Actions tab, where you can specify a custom version tag.

## Local Release

The `scripts/release.*` scripts handle build → test → packaging locally.
Run them before pushing a tag to verify everything passes on your machine.

### Linux / macOS

```sh
# Full release: configure, build, test, package
bash ./scripts/release.sh

# With explicit version
bash ./scripts/release.sh --version 0.2.0

# Skip tests (caution: only for smoke-testing packaging)
bash ./scripts/release.sh --skip-tests
```

Output: `dist/pp-<version>-<platform>.tar.gz` (Linux/macOS) or `.zip` (Windows under MSYS2).

### Windows (PowerShell)

```powershell
# Full release: configure, build, test, package
.\scripts\release.ps1

# With explicit version
.\scripts\release.ps1 -Version 0.2.0

# Skip tests
.\scripts\release.ps1 -SkipTests

# Also build the Inno Setup graphical installer
.\scripts\release.ps1 -Installer
```

Output: `dist\pp-<version>-windows-x64.zip`; with `-Installer` also
`dist\pp-setup-<version>.exe`.

## Platform-Native Installers

Use `scripts/package.*` to build graphical installers from an existing `dist/` binary.

### Windows (Inno Setup .exe)

Requires [Inno Setup](https://jrsoftware.org/isinfo.php).

```powershell
.\scripts\package.ps1                  # auto-detect version from CMakeLists.txt
.\scripts\package.ps1 -Version 0.2.0   # explicit version
.\scripts\package.ps1 -SkipBuild       # skip release build, use existing dist/pp.exe
```

The Inno Setup script is at `scripts/installer.iss`. It provides:

- Per-user install (no admin required).
- Start Menu shortcut.
- Optional PATH registration.
- Uninstall support (including PATH cleanup).

### Linux (.deb)

Requires `dpkg-deb` (from `dpkg-dev`).

```sh
bash ./scripts/package.sh                  # auto-detect platform
bash ./scripts/package.sh --version 0.2.0  # explicit version
bash ./scripts/package.sh --skip-build     # use existing dist/pp
```

Installing the resulting `.deb`:

```sh
sudo dpkg -i dist/pp_0.2.0_amd64.deb
sudo apt-get install -f  # if dependencies are needed
```

### macOS (.dmg)

Requires `hdiutil` (built-in on macOS).

```sh
bash ./scripts/package.sh                  # auto-detect platform
bash ./scripts/package.sh --version 0.2.0
bash ./scripts/package.sh --skip-build
```

Installing from the disk image:

```sh
open dist/pp-0.2.0.dmg
cp /Volumes/PromptEditor\ 0.2.0/pp /usr/local/bin/
```

## Release Checklist

Before tagging a release:

1. **Update version** in `CMakeLists.txt` (`project(PromptEditor VERSION X.Y.Z)`).
2. **Run release checks** at minimum:
   ```sh
   bash ./scripts/release.sh          # or .\scripts\release.ps1
   cmake --preset ninja-shared
   cmake --build --preset ninja-shared
   ctest --preset ninja-shared --output-on-failure
   ```
3. **Run package smoke tests** for installable libraries (see [cmake.md](cmake.md)).
4. **Update release notes** in the GitHub Release description.
5. **Tag and push**:
   ```sh
   git tag -a vX.Y.Z -m "Release vX.Y.Z"
   git push origin vX.Y.Z
   ```
6. **Verify** the CI release workflow completes successfully on all platforms.

## Verification

For detailed verification steps including sanitizer and shared-library presets,
see [cmake.md](cmake.md). A release-oriented local check typically covers:

| Preset | What it validates |
|--------|-------------------|
| `ninja-release` | Optimized build + test |
| `ninja-shared` | Shared-library build + test |
| `ninja-asan` | AddressSanitizer + UBSan |
| Package smoke | `find_package(PromptEditor)` consumption |
| Subproject smoke | `add_subdirectory()` consumption |

Do not publish releases from AI-assisted changes without maintainer approval.
