#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Script used for cross-platform comparison as part of the travis automation.
# Splits all test source code into multiple files, generates bytecode and
# uploads the bytecode into github.com/ethereum/solidity-test-bytecode where
# another travis job is triggered to do the actual comparison.
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
# (c) 2017 solidity contributors.
#------------------------------------------------------------------------------

set -e

SCRIPTDIR=$(dirname "$0")
SCRIPTDIR=$(realpath "${SCRIPTDIR}")


echo "Compiling all test contracts into bytecode..."
TMPDIR=$(mktemp -d)
(
    cd "${TMPDIR}"
    "${SCRIPTDIR}/isolate_tests.py" /src/test/

    cat > solc <<EOF
#!/usr/bin/env node
var process = require('process')
var fs = require('fs')

var compiler = require('/root/solc-js/wrapper.js')(require("${1}"))

for (var optimize of [false, true])
{
    for (var filename of process.argv.slice(2))
    {
        if (filename !== undefined)
        {
            var inputs = {}
            inputs[filename] = { content: fs.readFileSync(filename).toString() }
            var input = {
                language: 'Solidity',
                sources: inputs,
                settings: {
                    optimizer: { enabled: optimize },
                    outputSelection: { '*': { '*': ['*'] } }
                }
            }
            try {
                var result = JSON.parse(compiler.compile(JSON.stringify(input)))
                if (
                    !('contracts' in result) ||
                    Object.keys(result['contracts']).length === 0
                )
                {
                    // NOTE: do not exit here because this may be run on source which cannot be compiled
                    console.log(filename + ': ERROR')
                }
                else
                {
                    for (var outputName in result['contracts'])
                        for (var contractName in result['contracts'][outputName])
                        {
                            var contractData = result['contracts'][outputName][contractName];
                            if (contractData.evm !== undefined && contractData.evm.bytecode !== undefined)
                                console.log(filename + ':' + contractName + ' ' + contractData.evm.bytecode.object)
                            else
                                console.log(filename + ':' + contractName + ' NO BYTECODE')
                            console.log(filename + ':' + contractName + ' ' + contractData.metadata)
                        }
                }
            } catch (e) {
                console.log(filename + ': FATAL ERROR')
                console.error(filename)
                console.error(inputs)
            }
        }
    }
}
EOF
    chmod +x solc
    ./solc -- *.sol > /tmp/report.txt
)
rm -rf "$TMPDIR"
