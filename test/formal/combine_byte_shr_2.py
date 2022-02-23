from opcodes import BYTE, SHR, DIV
from rule import Rule
from z3 import BitVec, ULT

"""
byte(A, shr(B, X))
given A < B / 8
->
0
"""

rule = Rule()

n_bits = 256

# Input vars
X = BitVec('X', n_bits)
A = BitVec('A', n_bits)
B = BitVec('B', n_bits)

# Non optimized result
nonopt = BYTE(A, SHR(B, X))
# Optimized result
opt = 0

rule.require(ULT(A, DIV(B,8)))

rule.check(nonopt, opt)
