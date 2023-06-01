#!/usr/bin/env bash
set -eo pipefail

# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"
# shellcheck source=scripts/common_cmdline.sh
source "${REPO_ROOT}/scripts/common_cmdline.sh"

SOLTMPDIR=$(mktemp -d -t "cmdline-test-docs-examples-XXXXXX")
cd "$SOLTMPDIR"

"$REPO_ROOT"/scripts/isolate_tests.py "$REPO_ROOT"/docs/

developmentVersion=$("$REPO_ROOT/scripts/get_version.sh")

for f in *.yul *.sol
do
    # The contributors guide uses syntax tests, but we cannot
    # really handle them here.
    if grep -E 'DeclarationError:|// ----' "$f" >/dev/null
    then
        continue
    fi
    echo "    - Compiling example $f"

    opts=()
    # We expect errors if explicitly stated, or if imports
    # are used (in the style guide)
    if grep -E "// This will not compile" "$f" >/dev/null ||
        sed -e 's|//.*||g' "$f" | grep -E "import \"" >/dev/null
    then
        opts=(--expect-errors)
    fi
    if grep "// This will report a warning" "$f" >/dev/null
    then
        opts+=(--expect-warnings)
    fi
    if grep "// This may report a warning" "$f" >/dev/null
    then
        opts+=(--ignore-warnings)
    fi

    # Disable the version pragma in code snippets that only work with the current development version.
    # It's necessary because x.y.z won't match `^x.y.z` or `>=x.y.z` pragmas until it's officially released.
    sed -i.bak -E -e 's/pragma[[:space:]]+solidity[[:space:]]*(\^|>=)[[:space:]]*'"$developmentVersion"'/pragma solidity >0.0.1/' "$f"
    compileFull "${opts[@]}" "$SOLTMPDIR/$f"
done
rm -r "$SOLTMPDIR"
