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

# note that the following directories may be cached by circleci:
# - /usr/local/bin
# - /usr/local/sbin
# - /usr/local/lib
# - /usr/local/include
# - /usr/local/Cellar
# - /usr/local/Homebrew

set -eu

if [ ! -f /usr/local/lib/libz3.a ] # if this file does not exists (cache was not restored), rebuild dependencies
then
  git -C /usr/local/Homebrew/Library/Taps/homebrew/homebrew-core fetch --unshallow
  git -C /usr/local/Homebrew/Library/Taps/homebrew/homebrew-cask fetch --unshallow
  brew update
  brew unlink python
  brew install boost
  brew install cmake
  brew install wget
  brew install coreutils
  ./scripts/install_obsolete_jsoncpp_1_7_4.sh

  # z3
  z3_version="z3-4.8.12"
  osx_version="osx-10.15.7"
  wget "https://github.com/Z3Prover/z3/releases/download/$z3_version/$z3_version-x64-$osx_version.zip"
  unzip "$z3_version-x64-$osx_version.zip"
  rm -f "$z3_version-x64-$osx_version.zip"
  cp "$z3_version-x64-$osx_version/bin/libz3.a" /usr/local/lib
  cp "$z3_version-x64-$osx_version/bin/z3" /usr/local/bin
  cp "$z3_version-x64-$osx_version"/include/* /usr/local/include
  rm -rf "$z3_version-x64-$osx_version"

  # evmone
  wget https://github.com/ethereum/evmone/releases/download/v0.8.0/evmone-0.8.0-darwin-x86_64.tar.gz
  tar xzpf evmone-0.8.0-darwin-x86_64.tar.gz -C /usr/local
  rm -f evmone-0.8.0-darwin-x86_64.tar.gz

  # hera
  wget https://github.com/ewasm/hera/releases/download/v0.5.0/hera-0.5.0-darwin-x86_64.tar.gz
  tar xzpf hera-0.5.0-darwin-x86_64.tar.gz -C /usr/local
  rm -f hera-0.5.0-darwin-x86_64.tar.gz
fi
