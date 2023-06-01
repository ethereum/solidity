#!/usr/bin/env bash
set -euo pipefail

# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"
# shellcheck source=scripts/common_cmdline.sh
source "${REPO_ROOT}/scripts/common_cmdline.sh"

cd "$REPO_ROOT"/test/compilationTests/
for dir in */
do
    echo "    - $dir"
    cd "$dir"
    # shellcheck disable=SC2046 # These file names are not supposed to contain spaces.
    compileFull --expect-warnings $(find . -name '*.sol')
    cd ..
done
