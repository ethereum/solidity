from opcodes import SIGNEXTEND, SAR, SHR
from rule import Rule
from z3 import BitVec,  BitVecVal, ULE

"""
Rule:
SIGNEXTEND(A, SHR(B, X)) -> SAR(B, X)
given
    B % 8 == 0 AND
    A <= WordSize AND
    B <= wordSize AND
    (WordSize - B) / 8 == A + 1
"""

n_bits = 256

# Input vars
X = BitVec('X', n_bits)
Y = BitVec('Y', n_bits)
A = BitVec('A', n_bits)
B = BitVec('B', n_bits)

rule = Rule()
rule.require(B % 8 == 0)
rule.require(ULE(A, n_bits))
rule.require(ULE(B, n_bits))
rule.require((BitVecVal(n_bits, n_bits) - B) / 8 == A + 1)
rule.check(
    SIGNEXTEND(A, SHR(B, X)),
    SAR(B, X)
)
