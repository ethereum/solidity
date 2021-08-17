from rule import Rule
from opcodes import *

"""
Checking the implementation of SIGNEXTEND using Z3's native SignExt and Extract
"""

rule = Rule()
n_bits = 256

x = BitVec('X', n_bits)

def SIGNEXTEND_native(i, x):
    return SignExt(256 - 8 * i - 8, Extract(8 * i + 7, 0, x))

for i in range(0, 32):
    rule.check(
        SIGNEXTEND(BitVecVal(i, n_bits), x),
        SIGNEXTEND_native(i, x)
    )

i = BitVec('I', n_bits)
rule.require(UGT(i, BitVecVal(31, n_bits)))
rule.check(SIGNEXTEND(i, x), x)
