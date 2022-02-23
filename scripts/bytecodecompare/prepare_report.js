#!/usr/bin/env node
const process = require('process')
const fs = require('fs')

const compiler = require('solc')


function loadSource(sourceFileName, stripSMTPragmas)
{
    source = fs.readFileSync(sourceFileName).toString()

    if (stripSMTPragmas)
        // NOTE: replace() with string parameter replaces only the first occurrence.
        return source.replace('pragma experimental SMTChecker;', '');

    return source
}

function cleanString(string)
{
    if (string !== undefined)
        string = string.trim()
    return (string !== '' ? string : undefined)
}


let stripSMTPragmas = false
let firstFileArgumentIndex = 2

if (process.argv.length >= 3 && process.argv[2] === '--strip-smt-pragmas')
{
    stripSMTPragmas = true
    firstFileArgumentIndex = 3
}

for (const optimize of [false, true])
{
    for (const filename of process.argv.slice(firstFileArgumentIndex))
    {
        if (filename !== undefined)
        {
            let input = {
                language: 'Solidity',
                sources: {
                    [filename]: {content: loadSource(filename, stripSMTPragmas)}
                },
                settings: {
                    optimizer: {enabled: optimize},
                    outputSelection: {'*': {'*': ['evm.bytecode.object', 'metadata']}}
                }
            }
            if (!stripSMTPragmas)
                input['settings']['modelChecker'] = {engine: 'none'}

            let serializedOutput
            let result
            const serializedInput = JSON.stringify(input)

            let internalCompilerError = false
            try
            {
                serializedOutput = compiler.compile(serializedInput)
            }
            catch (exception)
            {
                internalCompilerError = true
            }

            if (!internalCompilerError)
            {
                result = JSON.parse(serializedOutput)

                if ('errors' in result)
                    for (const error of result['errors'])
                        // JSON interface still returns contract metadata in case of an internal compiler error while
                        // CLI interface does not. To make reports comparable we must force this case to be detected as
                        // an error in both cases.
                        if (['UnimplementedFeatureError', 'CompilerError', 'CodeGenerationError'].includes(error['type']))
                        {
                            internalCompilerError = true
                            break
                        }
            }

            if (
                internalCompilerError ||
                !('contracts' in result) ||
                Object.keys(result['contracts']).length === 0 ||
                Object.keys(result['contracts']).every(file => Object.keys(result['contracts'][file]).length === 0)
            )
                // NOTE: do not exit here because this may be run on source which cannot be compiled
                console.log(filename + ': <ERROR>')
            else
                for (const contractFile in result['contracts'])
                    for (const contractName in result['contracts'][contractFile])
                    {
                        const contractResults = result['contracts'][contractFile][contractName]

                        let bytecode = '<NO BYTECODE>'
                        let metadata = '<NO METADATA>'

                        if (
                            'evm' in contractResults &&
                            'bytecode' in contractResults['evm'] &&
                            'object' in contractResults['evm']['bytecode'] &&
                            cleanString(contractResults.evm.bytecode.object) !== undefined
                        )
                            bytecode = cleanString(contractResults.evm.bytecode.object)

                        if ('metadata' in contractResults && cleanString(contractResults.metadata) !== undefined)
                            metadata = contractResults.metadata

                        console.log(filename + ':' + contractName + ' ' + bytecode)
                        console.log(filename + ':' + contractName + ' ' + metadata)
                    }
        }
    }
}
