#!/usr/bin/env sh
# Release script — build, test, and package PromptEditor for distribution.
#
# Usage:
#   ./scripts/release.sh              # default: ninja-release preset
#   ./scripts/release.sh --version 0.2.0
#   ./scripts/release.sh --preset ninja-release --skip-tests
#
# Output: dist/pp-<version>-<platform>.tar.gz  (Linux/macOS)
#         dist/pp-<version>-<platform>.zip      (Windows)

set -eu

PRESET="ninja-release"
VERSION=""
SKIP_TESTS=0
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)

cd "$REPO_ROOT"

show_usage() {
  echo "Usage: $0 [options]"
  echo ""
  echo "Builds PromptEditor in release mode, runs tests, and packages the"
  echo "binary into a distributable archive under dist/."
  echo ""
  echo "Options:"
  echo "  --preset <name>    CMake preset (default: ninja-release)"
  echo "  --version <ver>    Version tag for the archive name (default: from CMake)"
  echo "  --skip-tests       Skip CTest after building"
  echo "  -h, --help         Show this help"
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --preset)
      if [ "$#" -lt 2 ]; then echo "ERROR: --preset requires a value." >&2; exit 2; fi
      PRESET="$2"; shift 2 ;;
    --version)
      if [ "$#" -lt 2 ]; then echo "ERROR: --version requires a value." >&2; exit 2; fi
      VERSION="$2"; shift 2 ;;
    --skip-tests)
      SKIP_TESTS=1; shift ;;
    -h|--help)
      show_usage; exit 0 ;;
    -*)
      echo "ERROR: unknown option $1" >&2; show_usage >&2; exit 2 ;;
    *)
      echo "ERROR: unexpected argument $1" >&2; show_usage >&2; exit 2 ;;
  esac
done

# Determine version from CMake project() if not provided
if [ -z "$VERSION" ]; then
  VERSION=$(sed -n 's/.*project(PromptEditor\s\+VERSION\s\+\([0-9.]\+\).*/\1/p' \
    "$REPO_ROOT/CMakeLists.txt" | head -1)
  if [ -z "$VERSION" ]; then
    VERSION="0.0.0"
  fi
fi

# Detect platform string
case "$(uname -s 2>/dev/null || echo unknown)" in
  Linux)   PLATFORM="linux" ;;
  Darwin)  PLATFORM="macos" ;;
  MINGW*|MSYS*|CYGWIN*)
    PLATFORM="windows"
    # Detect architecture for Windows
    if uname -m | grep -q "64"; then
      PLATFORM="windows-x64"
    else
      PLATFORM="windows-x86"
    fi
    ;;
  *)       PLATFORM="unknown" ;;
esac

# Build directory from preset name
BUILD_DIR="build/$PRESET"
DIST_DIR="$REPO_ROOT/dist"
ARCHIVE_NAME="pp-${VERSION}-${PLATFORM}"

echo "=== PromptEditor Release Build ==="
echo "  Preset:   $PRESET"
echo "  Version:  $VERSION"
echo "  Platform: $PLATFORM"
echo ""

# 1. Configure
echo "--- Configuring ---"
cmake --preset "$PRESET"

# 2. Build
echo ""
echo "--- Building ---"
cmake --build --preset "$PRESET"

# 3. Test (unless skipped)
if [ "$SKIP_TESTS" -eq 0 ]; then
  echo ""
  echo "--- Testing ---"
  ctest --preset "$PRESET" --output-on-failure
else
  echo ""
  echo "--- Skipping tests (--skip-tests) ---"
fi

# 4. Package
echo ""
echo "--- Packaging ---"
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"

BINARY_SRC="$BUILD_DIR/bin/pp"
case "$PLATFORM" in
  windows*)
    BINARY_SRC="$BINARY_SRC.exe"
    BINARY_DST="$DIST_DIR/pp.exe"
    cp "$BINARY_SRC" "$BINARY_DST"
    echo "  Binary: dist/pp.exe"

    # Create zip archive
    if command -v zip >/dev/null 2>&1; then
      (cd "$DIST_DIR" && zip -q "$ARCHIVE_NAME.zip" "pp.exe")
      echo "  Archive: dist/$ARCHIVE_NAME.zip"
    else
      echo "  Note: zip not found; skipping archive creation."
    fi
    ;;
  *)
    BINARY_DST="$DIST_DIR/pp"
    cp "$BINARY_SRC" "$BINARY_DST"
    chmod +x "$BINARY_DST"
    echo "  Binary: dist/pp"

    # Create tar.gz archive
    if command -v tar >/dev/null 2>&1; then
      tar -czf "$DIST_DIR/$ARCHIVE_NAME.tar.gz" -C "$DIST_DIR" "pp"
      echo "  Archive: dist/$ARCHIVE_NAME.tar.gz"
    else
      echo "  Note: tar not found; skipping archive creation."
    fi
    ;;
esac

echo ""
echo "=== Release complete ==="
echo "Output:  $DIST_DIR"
echo "Archive: $ARCHIVE_NAME"
