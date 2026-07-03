#!/usr/bin/env sh
set -eu

PRESET="ninja-debug"
ENABLE_TIDY=0
SKIP_FORMAT=0
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)

cd "$REPO_ROOT"

show_usage() {
  echo "Usage: $0 [--preset <name>] [--skip-format] [--enable-tidy]"
  echo "       $0 [preset] [tidy|--tidy]"
}

PRESET_SET=0
while [ "$#" -gt 0 ]; do
  case "$1" in
    --preset)
      if [ "$#" -lt 2 ]; then
        echo "ERROR: --preset requires a value." >&2
        exit 2
      fi
      PRESET="$2"
      PRESET_SET=1
      shift 2
      ;;
    --skip-format)
      SKIP_FORMAT=1
      shift
      ;;
    --enable-tidy|--tidy|tidy)
      ENABLE_TIDY=1
      shift
      ;;
    -h|--help)
      show_usage
      exit 0
      ;;
    -*)
      echo "ERROR: unknown option $1" >&2
      show_usage >&2
      exit 2
      ;;
    *)
      if [ "$PRESET_SET" -ne 0 ]; then
        echo "ERROR: unexpected argument $1" >&2
        show_usage >&2
        exit 2
      fi
      PRESET="$1"
      PRESET_SET=1
      shift
      ;;
  esac
done

if [ "$SKIP_FORMAT" -eq 0 ] && command -v clang-format >/dev/null 2>&1; then
  find include src tests -type f \( -name '*.c' -o -name '*.h' -o -name '*.cpp' \) -print0 \
    | xargs -0 clang-format --dry-run --Werror
elif [ "$SKIP_FORMAT" -eq 0 ]; then
  echo "Skipping clang-format: command not found."
fi

cmake --preset "$PRESET"
cmake --build --preset "$PRESET"

if [ "$ENABLE_TIDY" -ne 0 ]; then
  if command -v clang-tidy >/dev/null 2>&1; then
    BUILD_DIR="build/$PRESET"
    GENERATED_INCLUDE_DIR="$BUILD_DIR/generated/include"
    if [ -f "$BUILD_DIR/compile_commands.json" ]; then
      find src tests \( -path tests/package_smoke -o -path tests/subproject_smoke \) -prune -o -type f -name '*.c' \
        -exec clang-tidy --warnings-as-errors='*' -p "$BUILD_DIR" {} \;
    else
      find src tests \( -path tests/package_smoke -o -path tests/subproject_smoke \) -prune -o -type f -name '*.c' \
        -exec clang-tidy --warnings-as-errors='*' --extra-arg=-std=c11 {} -- -Iinclude -I"$GENERATED_INCLUDE_DIR" \;
    fi
  else
    echo "Skipping clang-tidy: command not found."
  fi
fi

ctest --preset "$PRESET" --output-on-failure
