from rule import Rule
from opcodes import *

"""
Rule:
1) SIGNEXTEND(A, X) -> X if A >= Pattern::WordSize / 8 - 1;

2) SIGNEXTEND(X, SIGNEXTEND(X, Y)) -> SIGNEXTEND(X, Y)

3) SIGNEXTEND(A, SIGNEXTEND(B, X)) -> SIGNEXTEND(min(A, B), X)
"""

n_bits = 128

# Input vars
X = BitVec('X', n_bits)
Y = BitVec('Y', n_bits)
A = BitVec('A', n_bits)
B = BitVec('B', n_bits)

rule1 = Rule()
# Requirements
rule1.require(UGE(A, BitVecVal(n_bits // 8 - 1, n_bits)))
rule1.check(SIGNEXTEND(A, X), X)

rule2 = Rule()
rule2.check(
    SIGNEXTEND(X, SIGNEXTEND(X, Y)),
    SIGNEXTEND(X, Y)
)

rule3 = Rule()
rule3.check(
    SIGNEXTEND(A, SIGNEXTEND(B, X)),
    SIGNEXTEND(If(ULT(A, B), A, B), X)
)

