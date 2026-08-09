#!/usr/bin/env bash
#
# new_project.sh - scaffold a new project from this template.
#
# Usage:
#   ./Scripts/new_project.sh <project-name> [destination-dir]
#
# Copies the template (excluding .git and build outputs), renames the CMake
# project and updates artifact references. The result builds as-is.
set -euo pipefail

if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "Usage: $0 <project-name> [destination-dir]" >&2
    exit 1
fi

NEW_NAME="$1"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Default destination: a sibling directory next to the template.
DEST_DIR="${2:-${SRC_DIR}/../${NEW_NAME}}"

if [[ ! "${NEW_NAME}" =~ ^[a-z0-9][a-z0-9_-]*$ ]]; then
    echo "Error: project name must match [a-z0-9][a-z0-9_-]* (got '${NEW_NAME}')" >&2
    exit 1
fi

if [[ -e "${DEST_DIR}" && -n "$(ls -A "${DEST_DIR}" 2>/dev/null)" ]]; then
    echo "Error: destination exists and is not empty: ${DEST_DIR}" >&2
    exit 1
fi

# Current project name in this template's CMakeLists.txt.
OLD_NAME="$(sed -n 's/^project(\([^ (]*\).*/\1/p' "${SRC_DIR}/CMakeLists.txt" | head -n 1)"
if [[ -z "${OLD_NAME}" ]]; then
    echo "Error: cannot detect project name in ${SRC_DIR}/CMakeLists.txt" >&2
    exit 1
fi

mkdir -p "${DEST_DIR}"
tar -C "${SRC_DIR}" \
    --exclude='.git' \
    --exclude='build*' \
    --exclude='cmake-build-*' \
    --exclude='.DS_Store' \
    -cf - . | tar -C "${DEST_DIR}" -xf -

# Rename project references in the copied files (literals only).
if [[ "${NEW_NAME}" != "${OLD_NAME}" ]]; then
    perl -pi -e "s/\Q${OLD_NAME}\E/${NEW_NAME}/g" \
        "${DEST_DIR}/CMakeLists.txt" \
        "${DEST_DIR}/README.md"
fi

echo "Project '${NEW_NAME}' created at: ${DEST_DIR}"
echo "Next steps:"
echo "  cd ${DEST_DIR}"
echo "  cmake --preset debug && cmake --build --preset debug"
