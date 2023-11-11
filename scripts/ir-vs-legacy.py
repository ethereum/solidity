import subprocess
from pathlib import Path
from re import findall

ir_optimized = []
legacy_optimized = []

for path in Path("test/libsolidity/semanticTests").rglob("*.sol"):
    fname = path.as_posix()
    file = open(fname)

    ir = []
    legacy = []
    for line in file:
        if "gas legacyOptimized" in line:
            legacy.extend([int(s) for s in findall(r'\b\d+\b', line)])
        if "gas irOptimized" in line:
            ir.extend([int(s) for s in findall(r'\b\d+\b', line)])

    if len(ir) > 0 and (len(ir) == len(legacy)):
        ir_optimized.extend(ir)
        legacy_optimized.extend(legacy)


ir = sum(ir_optimized)
legacy = sum(legacy_optimized)

print(ir - legacy)
