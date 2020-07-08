#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script to build the Solidity Sphinx documentation locally.
#
# The documentation for solidity is hosted at:
#
#     https://solidity.readthedocs.org
#
# ------------------------------------------------------------------------------
# SPDX-License-Identifier: GPL-3.0
#------------------------------------------------------------------------------

set -e
cd docs
pip3 install -r requirements.txt
sphinx-build -nW -b html -d _build/doctrees . _build/html
cd ..
