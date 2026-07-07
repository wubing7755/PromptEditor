#!/usr/bin/env sh
#
# package.sh — Build platform-native graphical installers from the release binary.
#
# On Linux:   produces a .deb package (Debian / Ubuntu)
# On macOS:   produces a .dmg disk image
#
# Prerequisites:
#   Linux:  dpkg-deb (from dpkg-dev)
#   macOS:  hdiutil (built-in)
#
# Usage:
#   ./scripts/package.sh                  # auto-detect platform
#   ./scripts/package.sh --version 0.2.0  # explicit version
#   ./scripts/package.sh --skip-build     # don't run release.sh first
#   ./scripts/package.sh --type deb       # force .deb (Linux only)
#   ./scripts/package.sh --type dmg       # force .dmg (macOS only)

set -eu

VERSION=""
DIST_DIR=""
SKIP_BUILD=0
PACKAGE_TYPE=""
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)

cd "$REPO_ROOT"

show_usage() {
  echo "Usage: $0 [options]"
  echo ""
  echo "Builds a platform-native graphical installer from the release binary."
  echo ""
  echo "Options:"
  echo "  --version <ver>   Override version (default: from CMakeLists.txt)"
  echo "  --type deb|dmg     Force package type (default: auto-detect)"
  echo "  --skip-build       Don't run release.sh; use existing dist/ binary"
  echo "  -h, --help         Show this help"
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --version)
      if [ "$#" -lt 2 ]; then echo "ERROR: --version requires a value." >&2; exit 2; fi
      VERSION="$2"; shift 2 ;;
    --type)
      if [ "$#" -lt 2 ]; then echo "ERROR: --type requires a value." >&2; exit 2; fi
      PACKAGE_TYPE="$2"; shift 2 ;;
    --skip-build)
      SKIP_BUILD=1; shift ;;
    -h|--help)
      show_usage; exit 0 ;;
    -*)
      echo "ERROR: unknown option $1" >&2; show_usage >&2; exit 2 ;;
    *)
      echo "ERROR: unexpected argument $1" >&2; show_usage >&2; exit 2 ;;
  esac
done

# Resolve version
if [ -z "$VERSION" ]; then
  VERSION=$(sed -n 's/.*project(PromptEditor\s\+VERSION\s\+\([0-9.]\+\).*/\1/p' \
    "$REPO_ROOT/CMakeLists.txt" | head -1)
  if [ -z "$VERSION" ]; then
    VERSION="0.0.0"
  fi
fi

# Resolve dist directory
if [ -z "$DIST_DIR" ]; then
  DIST_DIR="$REPO_ROOT/dist"
fi
mkdir -p "$DIST_DIR"

# Detect platform
OS=$(uname -s 2>/dev/null || echo unknown)
ARCH=$(uname -m 2>/dev/null || echo unknown)

if [ -z "$PACKAGE_TYPE" ]; then
  case "$OS" in
    Linux)  PACKAGE_TYPE="deb" ;;
    Darwin) PACKAGE_TYPE="dmg" ;;
    *)
      echo "ERROR: Unsupported platform: $OS" >&2
      echo "Use --type deb or --type dmg to specify manually." >&2
      exit 1
      ;;
  esac
fi

# Map architecture for Debian
case "$ARCH" in
  x86_64|amd64)  DEB_ARCH="amd64" ;;
  aarch64|arm64) DEB_ARCH="arm64" ;;
  *)             DEB_ARCH="$ARCH" ;;
esac

if [ "$PACKAGE_TYPE" = "deb" ] && [ "$OS" != "Linux" ]; then
  echo "WARNING: Building .deb on non-Linux system ($OS). dpkg-deb may not be available." >&2
fi
if [ "$PACKAGE_TYPE" = "dmg" ] && [ "$OS" != "Darwin" ]; then
  echo "ERROR: .dmg can only be built on macOS." >&2
  exit 1
fi

# Resolve binary
case "$PACKAGE_TYPE" in
  deb) BINARY_SRC="$DIST_DIR/pp" ;;
  dmg) BINARY_SRC="$DIST_DIR/pp" ;;
esac

# Ensure binary exists
if [ ! -f "$BINARY_SRC" ]; then
  if [ "$SKIP_BUILD" -eq 0 ]; then
    echo "Binary not found. Running release build..."
    "$SCRIPT_DIR/release.sh" --version "$VERSION"
  else
    echo "ERROR: Binary not found at $BINARY_SRC" >&2
    echo "Run ./scripts/release.sh first, or omit --skip-build." >&2
    exit 1
  fi
fi

echo "=== Build Package Installer ==="
echo "  Platform: $OS ($ARCH)"
echo "  Type:     $PACKAGE_TYPE"
echo "  Version:  $VERSION"
echo "  Binary:   $BINARY_SRC"
echo ""

# ──────────────────────────────────────────────────────────────────────
#  .deb  package  (Debian / Ubuntu)
# ──────────────────────────────────────────────────────────────────────
build_deb() {
  PKG_NAME="pp"
  PKG_DIR="$DIST_DIR/${PKG_NAME}_${VERSION}_${DEB_ARCH}"
  DEBIAN_DIR="$PKG_DIR/DEBIAN"
  BIN_DIR="$PKG_DIR/usr/bin"
  SHARE_DIR="$PKG_DIR/usr/share/doc/pp"

  echo "--- Building .deb package ---"
  rm -rf "$PKG_DIR"
  mkdir -p "$DEBIAN_DIR" "$BIN_DIR" "$SHARE_DIR"

  # control file
  cat > "$DEBIAN_DIR/control" << EOF
Package: pp
Version: ${VERSION}
Section: utils
Priority: optional
Architecture: ${DEB_ARCH}
Maintainer: PromptEditor Contributors
Homepage: https://github.com/wubing7755/PromptEditor
Description: Pure C prompt management CLI
 PromptEditor (pp) is a command-line tool for saving, organizing,
 retrieving, browsing, and improving prompt templates in a local
 file-backed library. It requires no database server and has zero
 external dependencies.
EOF

  # Binary
  cp "$BINARY_SRC" "$BIN_DIR/pp"
  chmod 755 "$BIN_DIR/pp"

  # Copyright
  cat > "$SHARE_DIR/copyright" << EOF
Format: https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/
Source: https://github.com/wubing7755/PromptEditor

Files: *
Copyright: $(date +%Y) PromptEditor Contributors
License: MIT
EOF

  # Build
  if command -v dpkg-deb >/dev/null 2>&1; then
    dpkg-deb --build "$PKG_DIR" "$DIST_DIR/${PKG_NAME}_${VERSION}_${DEB_ARCH}.deb"
    rm -rf "$PKG_DIR"
    local deb_file="$DIST_DIR/${PKG_NAME}_${VERSION}_${DEB_ARCH}.deb"
    local deb_size=$(du -h "$deb_file" | cut -f1)
    echo ""
    echo "  Package: $deb_file  (${deb_size})"
    echo ""
    echo "  To install on Ubuntu/Debian:"
    echo "    sudo dpkg -i ${PKG_NAME}_${VERSION}_${DEB_ARCH}.deb"
    echo "    sudo apt-get install -f  # if dependencies are needed"
  else
    echo "ERROR: dpkg-deb not found. Install dpkg-dev:" >&2
    echo "  sudo apt-get install dpkg-dev" >&2
    exit 1
  fi
}

# ──────────────────────────────────────────────────────────────────────
#  .dmg  disk image  (macOS)
# ──────────────────────────────────────────────────────────────────────
build_dmg() {
  DMG_NAME="pp-${VERSION}"
  DMG_DIR="$DIST_DIR/${DMG_NAME}.dmg"
  STAGING="$DIST_DIR/dmg-staging"

  echo "--- Building .dmg ---"
  rm -rf "$STAGING" "$DMG_DIR"
  mkdir -p "$STAGING"

  # Copy binary
  cp "$BINARY_SRC" "$STAGING/pp"
  chmod 755 "$STAGING/pp"

  # Create a README for the DMG window
  cat > "$STAGING/README.txt" << EOF
PromptEditor ${VERSION}

Installation:
  Drag the "pp" binary to /usr/local/bin to install system-wide, or to
  any directory on your PATH for per-user use.

  Terminal one-liner:
    cp pp /usr/local/bin/pp

Quick start:
  pp init
  pp add --title "Hello" --body "Your first prompt."
  pp list
EOF

  # Create Applications symlink for drag-to-install UX
  ln -s /Applications "$STAGING/Applications" 2>/dev/null || true

  # Build DMG
  if command -v hdiutil >/dev/null 2>&1; then
    hdiutil create -volname "PromptEditor ${VERSION}" \
      -srcfolder "$STAGING" \
      -ov -format UDZO \
      "$DMG_DIR" >/dev/null
    rm -rf "$STAGING"
    local dmg_size=$(du -h "$DMG_DIR" | cut -f1)
    echo ""
    echo "  Disk image: $DMG_DIR  (${dmg_size})"
    echo ""
    echo "  To install on macOS:"
    echo "    open ${DMG_NAME}.dmg"
    echo "    cp /Volumes/PromptEditor\\ ${VERSION}/pp /usr/local/bin/"
  else
    echo "ERROR: hdiutil not found. This script must run on macOS." >&2
    exit 1
  fi
}

# ──────────────────────────────────────────────────────────────────────
#  Dispatch
# ──────────────────────────────────────────────────────────────────────
case "$PACKAGE_TYPE" in
  deb) build_deb ;;
  dmg) build_dmg ;;
  *)
    echo "ERROR: Unknown package type: $PACKAGE_TYPE" >&2
    exit 1
    ;;
esac

echo ""
echo "=== Package ready ==="
echo "Upload this file to your GitHub Release."
