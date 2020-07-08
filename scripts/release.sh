#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script implementing release flow for solidity for Linux and macOS.
#
# TODO - At the time of writing, we only have ZIPs working.  Need to hook up
# support for Homebrew and PPAs.
#
# The documentation for solidity is hosted at:
#
#     https://solidity.readthedocs.org
#
# ------------------------------------------------------------------------------
# SPDX-License-Identifier: GPL-3.0
#------------------------------------------------------------------------------

ZIP_SUFFIX=$1
ZIP_TEMP_DIR=$(pwd)/build/zip/

# There is an implicit assumption here that we HAVE to run from root directory.
REPO_ROOT=$(pwd)

mkdir -p $ZIP_TEMP_DIR

# Copy all the solidity executables into a temporary directory prior to ZIP creation

cp $REPO_ROOT/build/solc/solc        $ZIP_TEMP_DIR

# For macOS, we run a fix-up script which alters all of the symbolic links within
# the executables and dynamic libraries such that the ZIP becomes self-contained, by
# revectoring all the dylib references to be relative to the directory containing the
# application, so that the ZIPs are self-contained, with the only external references
# being for kernel-level dylibs.

if [[ "$OSTYPE" == "darwin"* ]]; then
    python3 $REPO_ROOT/scripts/fix_homebrew_paths_in_standalone_zip.py $ZIP_TEMP_DIR
fi

# And ZIP it all up, with a filename suffix passed in on the command-line.
mkdir -p $REPO_ROOT/upload
zip -j $REPO_ROOT/upload/solidity-$ZIP_SUFFIX.zip $ZIP_TEMP_DIR/*
