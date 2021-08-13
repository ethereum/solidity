from rule import Rule
from opcodes import *

"""
Checks that the byte opcode (implemented using shift) is equivalent to a
canonical definition of byte using extract.
"""

rule = Rule()

n_bits = 256
x = BitVec('X', n_bits)

for i in range(0, 32):
    # For Byte, i = 0 corresponds to most significant bit
    # But for extract i = 0 corresponds to the least significant bit
    lsb = 31 - i
    rule.check(
        BYTE(BitVecVal(i, n_bits), x),
        Concat(BitVecVal(0, n_bits - 8), Extract(8 * lsb + 7, 8 * lsb, x))
    )
