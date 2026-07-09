#!/usr/bin/env sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
cd "$REPO_ROOT"

show_usage() {
  echo "Usage: $0 <PascalCaseName>"
  echo ""
  echo "Renames this template project in-place. Run once after cloning."
  echo ""
  echo "Example:"
  echo "  $0 MyNewLib"
  echo ""
  echo "The script derives the lowercase namespace (mylib) and UPPER_CASE"
  echo "macro prefix (MYLIB) from the PascalCase name automatically."
  echo ""
  echo "After renaming, run ./scripts/bootstrap.sh to verify the build."
  exit 0
}

if [ "$#" -ne 1 ]; then
  show_usage
fi

case "$1" in
  -h|--help) show_usage ;;
esac

PASCAL="$1"

# Validate: must start with uppercase letter, contain only alphanumeric chars
if ! echo "$PASCAL" | grep -qE '^[A-Z][A-Za-z0-9]*$'; then
  echo "ERROR: project name must start with an uppercase letter and contain only letters and digits." >&2
  echo "Example: MyNewLib" >&2
  exit 2
fi

LOWER=$(echo "$PASCAL" | tr '[:upper:]' '[:lower:]')
UPPER=$(echo "$LOWER" | tr '[:lower:]' '[:upper:]')

echo "Project rename:"
echo "  PascalCase: PromptLib -> ${PASCAL}"
echo "  lowercase:  promptlib        -> ${LOWER}"
echo "  UPPER_CASE: PROMPTLIB        -> ${UPPER}"
echo ""

# Find files containing any old identifier, excluding build/, .git/, and self
files=$(find . -type f \
  \( -name '*.c' -o -name '*.h' -o -name '*.cmake' -o -name '*.txt' \
     -o -name '*.json' -o -name '*.yml' -o -name '*.md' -o -name '*.in' \
     -o -name '*.cpp' -o -name '*.ps1' -o -name '*.sh' \) \
  ! -path './build/*' ! -path './.git/*' \
  ! -path './scripts/rename-project.sh' \
  ! -path './scripts/rename-project.ps1' \
  -exec grep -l -e 'PromptLib' -e 'promptlib' -e 'PROMPTLIB' -e 'PromptLib' {} + 2>/dev/null || true)

if [ -z "$files" ]; then
  echo "No files found with original project identifiers."
  echo "The project may already be renamed."
  exit 0
fi

updated=0
for file in $files; do
  count_pascal_std=$(grep -o 'PromptLib' "$file" 2>/dev/null | wc -l | tr -d ' ')
  count_lower=$(grep -o 'promptlib' "$file" 2>/dev/null | wc -l | tr -d ' ')
  count_upper=$(grep -o 'PROMPTLIB' "$file" 2>/dev/null | wc -l | tr -d ' ')
  count_pascal=$(grep -o 'PromptLib' "$file" 2>/dev/null | wc -l | tr -d ' ')

  sed -i.bak \
    -e "s/PromptLib/${PASCAL}/g" \
    -e "s/promptlib/${LOWER}/g" \
    -e "s/PROMPTLIB/${UPPER}/g" \
    -e "s/PromptLib/${PASCAL}/g" \
    "$file"
  rm -f "${file}.bak"

  echo "  $file"
  if [ "$count_pascal_std" -gt 0 ]; then
    echo "    PromptLib -> ${PASCAL} ($count_pascal_std)"
  fi
  if [ "$count_lower" -gt 0 ]; then
    echo "    promptlib -> ${LOWER} ($count_lower)"
  fi
  if [ "$count_upper" -gt 0 ]; then
    echo "    PROMPTLIB -> ${UPPER} ($count_upper)"
  fi
  # count_pascal includes PromptLib matches already shown above;
  # only print if there are additional standalone PromptLib matches
  extra_pascal=$((count_pascal - count_pascal_std))
  if [ "$extra_pascal" -gt 0 ]; then
    echo "    PromptLib -> ${PASCAL} ($extra_pascal)"
  fi
  updated=$((updated + 1))
done

echo ""
echo "Updated ${updated} files with new project identifiers."

# Rename directories
if [ -d "include/promptlib" ]; then
  mv "include/promptlib" "include/${LOWER}"
  echo "Renamed directory: include/promptlib -> include/${LOWER}"
fi

if [ -d "src/promptlib" ]; then
  mv "src/promptlib" "src/${LOWER}"
  echo "Renamed directory: src/promptlib -> src/${LOWER}"
fi

# Rename cmake files
if [ -f "cmake/PromptLibConfig.cmake.in" ]; then
  mv "cmake/PromptLibConfig.cmake.in" "cmake/${PASCAL}Config.cmake.in"
  echo "Renamed file: PromptLibConfig.cmake.in -> ${PASCAL}Config.cmake.in"
fi

if [ -f "cmake/promptlib_version.h.in" ]; then
  mv "cmake/promptlib_version.h.in" "cmake/${LOWER}_version.h.in"
  echo "Renamed file: promptlib_version.h.in -> ${LOWER}_version.h.in"
fi

echo ""
echo "Done. Run ./scripts/bootstrap.sh to verify the renamed project builds."
