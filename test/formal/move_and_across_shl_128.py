from rule import Rule
from opcodes import *

"""
Rule:
SHL(B, AND(X, A)) -> AND(SHL(B, X), A << B)
SHL(B, AND(A, X)) -> AND(SHL(B, X), A << B)
Requirements:
B < BitWidth
"""

rule = Rule()

n_bits = 128

# Input vars
X = BitVec('X', n_bits)
A = BitVec('A', n_bits)
B = BitVec('B', n_bits)

# Constants
BitWidth = BitVecVal(n_bits, n_bits)

# Requirements
rule.require(ULT(B, BitWidth))

# Non optimized result
nonopt_1 = SHL(B, AND(X, A))
nonopt_2 = SHL(B, AND(A, X))

# Optimized result
Mask = SHL(B, A)
opt = AND(SHL(B, X), Mask)

rule.check(nonopt_1, opt)
rule.check(nonopt_2, opt)
