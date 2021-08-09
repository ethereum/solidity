from rule import Rule
from opcodes import *

"""
Rule:
1) SHL(A, SIGNEXTEND(B, X)) -> SIGNEXTEND(A / 8 + B, SHL(A, B))
given A % 8 == 0 AND A / 8 + B >= B

2) SIGNEXTEND(A, SHR(B, X)) -> SAR(B, X)
given
    B % 8 == 0 AND
    A <= WordSize
    B <= wordSize
    (WordSize - B) / 8 == A + 1
"""

n_bits = 128

# Input vars
X = BitVec('X', n_bits)
Y = BitVec('Y', n_bits)
A = BitVec('A', n_bits)
B = BitVec('B', n_bits)

rule1 = Rule()
rule1.require(A % 8 == 0)
rule1.require(UGE(A / 8 + B, B))
rule1.check(
    SHL(A, SIGNEXTEND(B, X)),
    SIGNEXTEND(A / 8 + B, SHL(A, X))
)

rule2 = Rule()
rule2.require(B % 8 == 0)
rule2.require(ULE(A, n_bits))
rule2.require(ULE(B, n_bits))
rule2.require((BitVecVal(n_bits, n_bits) - B) / 8 == A + 1)
rule2.check(
    SIGNEXTEND(A, SHR(B, X)),
    SAR(B, X)
)

