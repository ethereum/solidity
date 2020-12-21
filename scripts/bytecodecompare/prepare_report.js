#!/usr/bin/env node
const process = require('process')
const fs = require('fs')

const compiler = require('./solc-js/wrapper.js')(require('./solc-js/soljson.js'))

for (const optimize of [false, true])
{
    for (const filename of process.argv.slice(2))
    {
        if (filename !== undefined)
        {
            const input = {
                language: 'Solidity',
                sources: {
                    [filename]: {content: fs.readFileSync(filename).toString()}
                },
                settings: {
                    optimizer: {enabled: optimize},
                    outputSelection: {'*': {'*': ['evm.bytecode.object', 'metadata']}},
                    "modelChecker": {"engine": "none"}
                }
            }

            const result = JSON.parse(compiler.compile(JSON.stringify(input)))
            if (
                !('contracts' in result) ||
                Object.keys(result['contracts']).length === 0 ||
                !result['contracts'][filename] ||
                Object.keys(result['contracts'][filename]).length === 0
            )
                // NOTE: do not exit here because this may be run on source which cannot be compiled
                console.log(filename + ': ERROR')
            else
                for (const contractName in result['contracts'][filename])
                {
                    const contractData = result['contracts'][filename][contractName];
                    if (contractData.evm !== undefined && contractData.evm.bytecode !== undefined)
                        console.log(filename + ':' + contractName + ' ' + contractData.evm.bytecode.object)
                    else
                        console.log(filename + ':' + contractName + ' NO BYTECODE')
                    console.log(filename + ':' + contractName + ' ' + contractData.metadata)
                }
        }
    }
}
