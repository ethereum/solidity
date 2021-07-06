#!/usr/bin/env bash

#------------------------------------------------------------------------------
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
robots_template_path="${repo_root}/docs/robots.txt.template"
robots_txt_path="${repo_root}/docs/_static/robots.txt"
latest_version=$("${repo_root}/scripts/get_version.sh")

sed -E -e "s/\{\{[[:space:]]LATEST_VERSION[[:space:]]\}\}/${latest_version}/g" "$robots_template_path" > "$robots_txt_path"
