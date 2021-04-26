#!/usr/bin/env bash

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
# (c) 2021 solidity contributors.
#------------------------------------------------------------------------------

set -eu

repo_root="$(dirname "$0")/.."
robots_txt_path="${repo_root}/docs/_static/robots.txt"
version_from_script=$("${repo_root}"/scripts/get_version.sh | tr -d '\n')
last_line_robots=$(tail -n 1 "${robots_txt_path}")
version_number_in_robots="#(\w+):\s\/en\/v([0-9]\.[0-9]\.[0-9])\/";
if [[ "$last_line_robots" =~ ${version_number_in_robots} ]]; then
    if [[ ${BASH_REMATCH[1]} != "Disallow" ]]; then
        printf "Error: The last line of robots.txt does not match the expected 'Disallow' directive.\n"
        exit 1
    fi
    if [[ ${BASH_REMATCH[2]} != "$version_from_script" ]]; then
        sed -i.bak '$d' "$robots_txt_path"
        echo "Disallow: /en/v${BASH_REMATCH[2]}/" >> "$robots_txt_path"
        echo "#Disallow: /en/v${version_from_script}/" >> "$robots_txt_path"
    fi
fi
