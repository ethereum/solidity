from rule import Rule
from opcodes import *

"""
Rule:
AND(A, SIGNEXTEND(B, X)) -> AND(A, X)
given
    B < WordSize / 8 - 1 AND
    A & (1 << ((B + 1) * 8) - 1) == A
"""

n_bits = 128

# Input vars
X = BitVec('X', n_bits)
A = BitVec('A', n_bits)
B = BitVec('B', n_bits)

rule = Rule()
# Requirements
rule.require(ULT(B, BitVecVal(n_bits // 8 - 1, n_bits)))
rule.require((A & ((BitVecVal(1, n_bits) << ((B + 1) * 8)) - 1)) == A)
rule.check(
    AND(A, SIGNEXTEND(B, X)),
    AND(A, X)
)
