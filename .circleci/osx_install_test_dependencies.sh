#! /bin/bash
#------------------------------------------------------------------------------
# Bash script to install osx dependencies
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
# (c) 2016-2019 solidity contributors.
# ------------------------------------------------------------------------------

set -eu

function validate_checksum {
    local package="$1"
    local expected_checksum="$2"

    local actual_checksum
    actual_checksum=$(sha256sum "$package")
    if [[ $actual_checksum != "${expected_checksum}  ${package}" ]]
    then
        >&2 echo "ERROR: Wrong checksum for package $package."
        >&2 echo "Actual:   $actual_checksum"
        >&2 echo "Expected: $expected_checksum"
        exit 1
    fi
}

# Disable automatic `brew cleanup` after every install command. Unnecessary on CI machines.
export HOMEBREW_NO_INSTALL_CLEANUP=1
# JRE is required to run eldarica solver
brew install \
    wget \
    openjdk@11 \
    unzip

# eldarica
eldarica_version="2.1"
wget "https://github.com/uuverifiers/eldarica/releases/download/v${eldarica_version}/eldarica-bin-${eldarica_version}.zip" -O /tmp/eld_binaries.zip
validate_checksum /tmp/eld_binaries.zip 0ac43f45c0925383c9d2077f62bbb515fd792375f3b2b101b30c9e81dcd7785c
unzip /tmp/eld_binaries.zip -d /tmp
sudo mv /tmp/eldarica/{eld,eld-client,target,eldEnv} /usr/local/bin
rm -rf /tmp/{eldarica,eld_binaries.zip}

#cvc5
cvc5_version="1.2.0"
cvc5_archive_name="cvc5-macOS-arm64-static"
wget "https://github.com/cvc5/cvc5/releases/download/cvc5-${cvc5_version}/${cvc5_archive_name}.zip" -O /tmp/cvc5.zip
validate_checksum /tmp/cvc5.zip 57d2d4855af3f3865110a254e415098b4e150a655f297010e27eb292f48f7da7
sudo unzip -j /tmp/cvc5.zip "${cvc5_archive_name}/bin/cvc5" -d /usr/local/bin
rm -f /tmp/cvc5.zip

# z3
z3_version="4.13.3"
z3_archive_name="z3-${z3_version}-arm64-osx-13.7"
wget "https://github.com/Z3Prover/z3/releases/download/z3-${z3_version}/${z3_archive_name}.zip" -O /tmp/z3.zip
validate_checksum /tmp/z3.zip 2b2c6e23ff5488722bc93002e04c6d6f60d621af9d345f2e05a8691b40280e53
sudo unzip -j /tmp/z3.zip "${z3_archive_name}/bin/z3" -d /usr/local/bin
rm -f /tmp/z3.zip

# evmone
evmone_version="0.22.0"
evmone_package="evmone-${evmone_version}-darwin-arm64.tar.gz"
wget "https://github.com/ipsilon/evmone/releases/download/v${evmone_version}/${evmone_package}"
validate_checksum "$evmone_package" 3ff5633e49ae3726dc094c7c9819440c11d04c8036bc27f45a4385120951599c
sudo tar xzpf "$evmone_package" -C /usr/local
rm "$evmone_package"
