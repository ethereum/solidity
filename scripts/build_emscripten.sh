#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script for building Solidity for emscripten.
#
# The documentation for solidity is hosted at:
#
#     https://docs.soliditylang.org
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
# (c) 2016 solidity contributors.
#------------------------------------------------------------------------------

set -e

params=""
if (( $# != 0 )); then
    params="$(printf "%q " "${@}")"
fi

# solbuildpackpusher/solidity-buildpack-deps:emscripten-16
# NOTE: Without `safe.directory` git would assume it's not safe to operate on /root/project since it's owned by a different user.
# See https://github.blog/2022-04-12-git-security-vulnerability-announced/
docker run -v "$(pwd):/root/project" -w /root/project \
    solbuildpackpusher/solidity-buildpack-deps@sha256:19fcb5ac029bbc27ec36e10f7d14ea224d8010145f9690562ef084fd16146b0c \
    /bin/bash -c "git config --global --add safe.directory /root/project && ./scripts/ci/build_emscripten.sh ${params}"
