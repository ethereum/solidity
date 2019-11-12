#! /bin/bash
#------------------------------------------------------------------------------
# Bash script to install osx dependencies
#
# The documentation for solidity is hosted at:
#
#     https://solidity.readthedocs.org
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

if [ ! -f /usr/local/lib/libz3.a ] # if this file does not exists (cache was not restored), rebuild dependencies
then
  brew unlink python
  brew install boost
  brew install cmake
  brew install wget
  brew install coreutils
  ./scripts/install_obsolete_jsoncpp_1_7_4.sh

  # z3
  wget https://github.com/Z3Prover/z3/releases/download/z3-4.8.6/z3-4.8.6-x64-osx-10.14.6.zip
  unzip z3-4.8.6-x64-osx-10.14.6.zip
  rm -f z3-4.8.6-x64-osx-10.14.6.zip
  cp z3-4.8.6-x64-osx-10.14.6/bin/libz3.a /usr/local/lib
  cp z3-4.8.6-x64-osx-10.14.6/bin/z3 /usr/local/bin
  cp z3-4.8.6-x64-osx-10.14.6/include/* /usr/local/include
  rm -rf z3-4.8.6-x64-osx-10.14.6

  # evmone
  wget https://github.com/ethereum/evmone/releases/download/v0.1.0/evmone-0.1.0-darwin-x86_64.tar.gz
  tar xzpf evmone-0.1.0-darwin-x86_64.tar.gz -C /usr/local
  rm -f evmone-0.1.0-darwin-x86_64.tar.gz
fi

