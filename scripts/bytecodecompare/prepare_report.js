#!/usr/bin/env node
var process = require('process')
var fs = require('fs')

var compiler = require('./solc-js/wrapper.js')(require('./solc-js/soljson.js'))

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
                    outputSelection: { '*': { '*': ['evm.bytecode.object', 'metadata'] } },
                    "modelChecker": { "engine": "none" }
                }
            }
            var result = JSON.parse(compiler.compile(JSON.stringify(input)))
            if (
                !('contracts' in result) ||
                Object.keys(result['contracts']).length === 0 ||
                !result['contracts'][filename] ||
                Object.keys(result['contracts'][filename]).length === 0
            )
            {
                // NOTE: do not exit here because this may be run on source which cannot be compiled
                console.log(filename + ': ERROR')
            }
            else
            {
                for (var contractName in result['contracts'][filename])
                {
                    var contractData = result['contracts'][filename][contractName];
                    if (contractData.evm !== undefined && contractData.evm.bytecode !== undefined)
                        console.log(filename + ':' + contractName + ' ' + contractData.evm.bytecode.object)
                    else
                        console.log(filename + ':' + contractName + ' NO BYTECODE')
                    console.log(filename + ':' + contractName + ' ' + contractData.metadata)
                }
            }
        }
    }
}
