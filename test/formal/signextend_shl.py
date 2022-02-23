from opcodes import SHL, SIGNEXTEND
from rule import Rule
from z3 import BitVec, LShR, ULE

"""
Rule:
SHL(A, SIGNEXTEND(B, X)) -> SIGNEXTEND((A >> 3) + B, SHL(A, X))
given return A & 7 == 0  AND  A <= WordSize  AND  B <= WordSize / 8
"""

n_bits = 256

# Input vars
X = BitVec('X', n_bits)
Y = BitVec('Y', n_bits)
A = BitVec('A', n_bits)
B = BitVec('B', n_bits)

rule = Rule()
rule.require(A & 7 == 0)
rule.require(ULE(A, n_bits))
rule.require(ULE(B, n_bits / 8))
rule.check(
    SHL(A, SIGNEXTEND(B, X)),
    SIGNEXTEND(LShR(A, 3) + B, SHL(A, X))
)
