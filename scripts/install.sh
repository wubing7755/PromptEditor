#!/usr/bin/env sh
# Install script — copy PromptLib binary to a user-chosen location
# and optionally add it to PATH.
#
# Usage:
#   ./scripts/install.sh                          # interactive
#   ./scripts/install.sh --prefix /usr/local      # system-wide
#   ./scripts/install.sh --prefix ~/.local/bin    # per-user
#   ./scripts/install.sh --prefix /opt/pp --no-path

set -eu

PREFIX=""
ADD_PATH="prompt"
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
DEFAULT_PREFIX=""

# Detect default install prefix
case "$(uname -s 2>/dev/null || echo unknown)" in
  Linux|Darwin)
    # Prefer ~/.local/bin for per-user installs
    if [ -n "${HOME:-}" ] && [ -d "$HOME/.local/bin" ]; then
      DEFAULT_PREFIX="$HOME/.local/bin"
    elif [ -w "/usr/local/bin" ]; then
      DEFAULT_PREFIX="/usr/local/bin"
    else
      DEFAULT_PREFIX="${HOME:-~}/.local/bin"
    fi
    BINARY_NAME="pp"
    ;;
  MINGW*|MSYS*|CYGWIN*)
    DEFAULT_PREFIX="${LOCALAPPDATA:-$HOME/AppData/Local}/Programs/PromptLib"
    BINARY_NAME="pp.exe"
    ;;
  *)
    DEFAULT_PREFIX="${HOME:-~}/.local/bin"
    BINARY_NAME="pp"
    ;;
esac

show_usage() {
  echo "Usage: $0 [options]"
  echo ""
  echo "Installs the PromptLib (pp) binary to the specified location."
  echo ""
  echo "Options:"
  echo "  --prefix <path>   Install directory (default: $DEFAULT_PREFIX)"
  echo "  --no-path         Skip the PATH setup prompt"
  echo "  --force           Overwrite existing binary without asking"
  echo "  -h, --help        Show this help"
}

NO_PATH=0
FORCE=0

while [ "$#" -gt 0 ]; do
  case "$1" in
    --prefix)
      if [ "$#" -lt 2 ]; then echo "ERROR: --prefix requires a value." >&2; exit 2; fi
      PREFIX="$2"; shift 2 ;;
    --no-path)
      NO_PATH=1; shift ;;
    --force)
      FORCE=1; shift ;;
    -h|--help)
      show_usage; exit 0 ;;
    -*)
      echo "ERROR: unknown option $1" >&2; show_usage >&2; exit 2 ;;
    *)
      echo "ERROR: unexpected argument $1" >&2; show_usage >&2; exit 2 ;;
  esac
done

if [ -z "$PREFIX" ]; then
  PREFIX="$DEFAULT_PREFIX"
fi

# Determine the source binary
BINARY_SRC="$REPO_ROOT/build/ninja-release/bin/$BINARY_NAME"
if [ ! -f "$BINARY_SRC" ]; then
  BINARY_SRC="$REPO_ROOT/build/ninja-debug/bin/$BINARY_NAME"
fi
if [ ! -f "$BINARY_SRC" ]; then
  echo "ERROR: Could not find pp binary." >&2
  echo "Run ./scripts/release.sh first, or build with:" >&2
  echo "  cmake --preset ninja-release && cmake --build --preset ninja-release" >&2
  exit 1
fi

echo "=== PromptLib Install ==="
echo "  Source:  $BINARY_SRC"
echo "  Prefix:  $PREFIX"
echo ""

# Check for existing binary
DEST="$PREFIX/$BINARY_NAME"
if [ -f "$DEST" ] && [ "$FORCE" -eq 0 ]; then
  printf "Binary already exists at %s. Overwrite? [y/N] " "$DEST"
  read -r answer
  case "$answer" in
    [Yy]|[Yy][Ee][Ss]) ;;
    *) echo "Installation cancelled."; exit 0 ;;
  esac
fi

# Create destination directory if needed
mkdir -p "$PREFIX"

# Copy binary
cp "$BINARY_SRC" "$DEST"
chmod +x "$DEST" 2>/dev/null || true
echo "Installed: $DEST"

# Verify
echo ""
echo "--- Verification ---"
"$DEST" --version

# PATH guidance
if [ "$NO_PATH" -eq 0 ]; then
  echo ""
  echo "--- PATH Setup ---"
  case "$(uname -s 2>/dev/null || echo unknown)" in
    Linux|Darwin)
      case "$PREFIX" in
        /usr/local/bin|/usr/bin|/bin)
          echo "Installed to a system path; pp should be available immediately." ;;
        *)
          if echo "$PATH" | tr ':' '\n' | grep -qxF "$PREFIX"; then
            echo "$PREFIX is already on your PATH."
          else
            SHELL_RC=""
            case "${SHELL:-}" in
              */zsh)  SHELL_RC="$HOME/.zshrc" ;;
              */bash) SHELL_RC="$HOME/.bashrc" ;;
              *)      SHELL_RC="$HOME/.profile" ;;
            esac
            echo "Add the following to your $SHELL_RC to use pp globally:"
            echo ""
            echo "  export PATH=\"$PREFIX:\$PATH\""
            echo ""
            printf "Add this line now? [y/N] "
            read -r answer
            case "$answer" in
              [Yy]|[Yy][Ee][Ss])
                echo "export PATH=\"$PREFIX:\$PATH\"" >> "$SHELL_RC"
                echo "Added to $SHELL_RC. Restart your shell or run:"
                echo "  source $SHELL_RC"
                ;;
              *) echo "Skipped. You can add it manually later." ;;
            esac
          fi
          ;;
      esac
      ;;
    MINGW*|MSYS*|CYGWIN*)
      echo "On Windows (MSYS2/Git Bash), add the following to your ~/.bashrc:"
      echo ""
      echo "  export PATH=\"$PREFIX:\$PATH\""
      echo ""
      echo "For native Windows CMD or PowerShell, add $PREFIX"
      echo "to your user PATH via System Environment Variables."
      ;;
  esac
fi

echo ""
echo "=== Install complete ==="
