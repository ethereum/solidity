#!/usr/bin/env node
const process = require('process')
const fs = require('fs')

const compiler = require('solc')

SETTINGS_PRESETS = {
    'legacy-optimize':    {optimize: true,  viaIR: false},
    'legacy-no-optimize': {optimize: false, viaIR: false},
    'via-ir-optimize':    {optimize: true,  viaIR: true},
    'via-ir-no-optimize': {optimize: false, viaIR: true},
}

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

let inputFiles = []
let stripSMTPragmas = false
let presets = []

for (let i = 2; i < process.argv.length; ++i)
{
    if (process.argv[i] === '--strip-smt-pragmas')
        stripSMTPragmas = true
    else if (process.argv[i] === '--preset')
    {
        if (i + 1 === process.argv.length)
            throw Error("Option --preset was used, but no preset name given.")

        presets.push(process.argv[i + 1])
        ++i;
    }
    else
        inputFiles.push(process.argv[i])
}

if (presets.length === 0)
    presets = ['legacy-no-optimize', 'legacy-optimize']

for (const preset of presets)
    if (!(preset in SETTINGS_PRESETS))
        throw Error(`Invalid preset name: ${preset}.`)

for (const preset of presets)
{
    settings = SETTINGS_PRESETS[preset]

    for (const filename of inputFiles)
    {
        let input = {
            language: 'Solidity',
            sources: {
                [filename]: {content: loadSource(filename, stripSMTPragmas)}
            },
            settings: {
                optimizer: {enabled: settings.optimize},
                // NOTE: We omit viaIR rather than set it to false to handle older versions that don't have it.
                viaIR: settings.viaIR ? true : undefined,
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
                    if (['UnimplementedFeatureError', 'CompilerError', 'CodeGenerationError', 'YulException'].includes(error['type']))
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
