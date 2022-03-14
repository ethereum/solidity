#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Script that tests that the compiler works correctly regardless of the locale
# setting. As a prerequisite, the following locales must be enabled system-wide:
# C, tr_TR.utf8, ja_JP.eucjp.
#
# Usage:
#    <script name>.sh <path to solc binary>
#
# ------------------------------------------------------------------------------
# This file is part of solidity.
#
# solidity is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# solidity is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with solidity.  If not, see <http://www.gnu.org/licenses/>
#
# (c) 2022 solidity contributors.
#------------------------------------------------------------------------------

set -eo pipefail

REPO_ROOT=$(cd "$(dirname "$0")/.." && pwd)
# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"

solc_binary="$1"
(( $# == 1 )) || fail "Expected exactly 1 argument."

# This test won't work without some specific locales installed
locale -a | grep -e "^tr_TR\.utf8$" || fail "Locale 'tr_TR.utf8' not available."
locale -a | grep -e "^ja_JP\.eucjp$" || fail "Locale 'ja_JP.eucjp' not available."
locale -a | grep -e "^C$" || fail "Locale 'C' not available."
locale -a | grep -e "^__invalid_locale__$" && fail "'__invalid_locale__' is not supposed to be a valid locale name."

i="i"

test_code=$(cat <<'EOF'
    // SPDX-License-Identifier: GPL-3.0
    pragma solidity *;
    library L {}
EOF
)

# Whatever locale is set by default.
printTask "Testing the default locale..."
default_locale_output=$(echo "$test_code" | "$solc_binary" - --bin)

# Plain C locale
printTask "Testing the C locale..."
export LC_ALL=C
[[ ${i^^} == "I" ]] || assertFail
c_locale_output=$(echo "$test_code" | "$solc_binary" - --bin)
diff_values "$default_locale_output" "$c_locale_output"

# Turkish locale, which has capitalization rules (`i` -> `İ` and `I` to `ı`) that can make identifiers invalid.
printTask "Testing the Turkish locale..."
export LC_ALL=tr_TR.utf8
[[ ${i^^} != "I" ]] || assertFail
tr_locale_output=$(echo "$test_code" | "$solc_binary" - --bin)
diff_values "$default_locale_output" "$tr_locale_output"

# A different locale, that should not do anything special to ASCII chars.
printTask "Testing the Japanese locale..."
export LC_ALL=ja_JP.eucjp
[[ ${i^^} == "I" ]] || assertFail
ja_locale_output=$(echo "$test_code" | "$solc_binary" - --bin)
diff_values "$default_locale_output" "$ja_locale_output"

# The compiler should not crash if the locale is not valid.
printTask "Testing an invalid locale..."
export LC_ALL=__invalid_locale__ || true
invalid_locale_output=$(echo "$test_code" | "$solc_binary" - --bin)
diff_values "$default_locale_output" "$invalid_locale_output"
