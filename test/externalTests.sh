#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script to run external Solidity tests.
#
# Argument: Path to soljson.js to test.
#
# Requires npm, networking access and git to download the tests.
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

if [ ! -f "$1" ]
then
  echo "Usage: $0 <path to soljson.js>"
  exit 1
fi

SOLJSON="$1"

DIR=$(mktemp -d)
(
    echo "Running Zeppelin tests..."
    git clone --depth 1 https://github.com/OpenZeppelin/zeppelin-solidity.git "$DIR"
    cd "$DIR"

    # Fix some things that look like bugs (only seemed to fail on Node 6 and not Node 8)
    # FIXME: report upstream or to web3.js?
    sed -i -e 's/let token = await ERC827TokenMock.new();//;' test/token/ERC827/ERC827Token.js
    sed -i -e 's/CappedCrowdsale.new(this.startTime, this.endTime, rate, wallet, 0)/CappedCrowdsale.new(this.startTime, this.endTime, rate, wallet, 0, this.token.address)/' test/crowdsale/CappedCrowdsale.test.js
    sed -i -e 's/RefundableCrowdsale.new(this.startTime, this.endTime, rate, wallet, 0, { from: owner })/RefundableCrowdsale.new(this.startTime, this.endTime, rate, wallet, 0, this.token.address, { from: owner })/' test/crowdsale/RefundableCrowdsale.test.js

    npm install
    find . -name soljson.js -exec cp "$SOLJSON" {} \;

    npm run test
)
rm -rf "$DIR"
