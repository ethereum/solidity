#!/usr/bin/env python3

import sys
import glob
import subprocess
import json

SOLC_BIN = sys.argv[1]
REPORT_FILE = open("report.txt", mode="w", encoding='utf8', newline='\n')

for optimize in [False, True]:
    for f in sorted(glob.glob("*.sol")):
        sources = {}
        sources[f] = {'content': open(f, mode='r', encoding='utf8').read()}
        input_json = {
            'language': 'Solidity',
            'sources': sources,
            'settings': {
                'optimizer': {
                    'enabled': optimize
                },
                'outputSelection': {'*': {'*': ['evm.bytecode.object', 'metadata']}},
                'modelChecker': { "engine": 'none' }
            }
        }
        args = [SOLC_BIN, '--standard-json']
        if optimize:
            args += ['--optimize']
        proc = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (out, err) = proc.communicate(json.dumps(input_json).encode('utf-8'))

        result = json.loads(out.decode('utf-8').strip())
        if (
            'contracts' not in result or
            len(result['contracts']) == 0 or
            all(len(file_results) == 0 for file_name, file_results in result['contracts'].items())
        ):
            REPORT_FILE.write(f + ": ERROR\n")
        else:
            for filename in sorted(result['contracts'].keys()):
                for contractName in sorted(result['contracts'][filename].keys()):
                    bytecode = result['contracts'][filename][contractName].get('evm', {}).get('bytecode', {}).get('object', 'NO BYTECODE')
                    metadata = result['contracts'][filename][contractName]['metadata']

                    REPORT_FILE.write(filename + ':' + contractName + ' ' + bytecode + '\n')
                    REPORT_FILE.write(filename + ':' + contractName + ' ' + metadata + '\n')
