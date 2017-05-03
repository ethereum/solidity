#!/usr/bin/env python

import sys
import glob
import subprocess
import json

solc = sys.argv[1]
report = open("report.txt", "w")

for optimize in [False, True]:
    for f in sorted(glob.glob("*.sol")):
        args = [solc, '--combined-json', 'bin,metadata', f]
        if optimize:
            args += ['--optimize']
        proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (out, err) = proc.communicate()
        try:
            result = json.loads(out.strip())
            for contractName in sorted(result['contracts'].keys()):
                report.write(contractName + ' ' + result['contracts'][contractName]['bin'] + '\n')
                report.write(contractName + ' ' + result['contracts'][contractName]['metadata'] + '\n')
        except:
            report.write(f + ": ERROR\n")
