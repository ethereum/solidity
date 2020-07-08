#! /bin/bash
#------------------------------------------------------------------------------
# Bash script to install osx dependencies
#
# The documentation for solidity is hosted at:
#
#     https://solidity.readthedocs.org
#
# ------------------------------------------------------------------------------
# SPDX-License-Identifier: GPL-3.0
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
  wget https://github.com/Z3Prover/z3/releases/download/z3-4.8.8/z3-4.8.8-x64-osx-10.14.6.zip
  unzip z3-4.8.8-x64-osx-10.14.6.zip
  rm -f z3-4.8.8-x64-osx-10.14.6.zip
  cp z3-4.8.8-x64-osx-10.14.6/bin/libz3.a /usr/local/lib
  cp z3-4.8.8-x64-osx-10.14.6/bin/z3 /usr/local/bin
  cp z3-4.8.8-x64-osx-10.14.6/include/* /usr/local/include
  rm -rf z3-4.8.8-x64-osx-10.14.6

  # evmone
  wget https://github.com/ethereum/evmone/releases/download/v0.4.0/evmone-0.4.0-darwin-x86_64.tar.gz
  tar xzpf evmone-0.4.0-darwin-x86_64.tar.gz -C /usr/local
  rm -f evmone-0.4.0-darwin-x86_64.tar.gz
fi

