#!/usr/bin/env python3

import sys
import glob
import subprocess
import json

SOLC_BIN = sys.argv[1]
REPORT_FILE = open("report.txt", mode="w", encoding='utf8', newline='\n')

for optimize in [False, True]:
    for f in sorted(glob.glob("*.sol")):
        for output_type in [['evm.bytecode.object', 'metadata'], ['ewasm.wast', 'ewasm.wasm', 'metadata']]:
            sources = {}
            sources[f] = {'content': open(f, mode='r', encoding='utf8').read()}
            input_json = {
                'language': 'Solidity',
                'sources': sources,
                'settings': {
                    'optimizer': {
                        'enabled': optimize
                    },
                    'outputSelection': {'*': {'*': output_type}},
                    'modelChecker': {"engine": 'none'}
                }
            }
            args = [SOLC_BIN, '--standard-json']
            if optimize:
                args += ['--optimize']
            proc = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            (out, err) = proc.communicate(json.dumps(input_json).encode('utf-8'))
            try:
                result = json.loads(out.decode('utf-8').strip())
                for filename in sorted(result['contracts'].keys()):
                    for contractName in sorted(result['contracts'][filename].keys()):
                        outputType = ""
                        if 'ewasm.wast' in output_type:
                            outputType = "(ewasm)"
                        contractData = result['contracts'][filename][contractName]
                        if 'evm' in contractData and 'bytecode' in contractData['evm']:
                            REPORT_FILE.write(filename + ':' + contractName + ' ' +
                                              contractData['evm']['bytecode']['object'] + '\n')
                        elif 'ewasm' in contractData and 'wasm' in contractData['ewasm']:
                            REPORT_FILE.write(filename + ':' + contractName + outputType +
                                              contractData['ewasm']['wasm'] + '\n')
                        else:
                            REPORT_FILE.write(filename + ':' + contractName + outputType + ' NO BYTECODE\n')
                        REPORT_FILE.write(filename + ':' + contractName + outputType + ' ' + contractData['metadata'] + '\n')
            except KeyError:
                REPORT_FILE.write(f + ": ERROR\n")
