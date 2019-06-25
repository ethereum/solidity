from rule import Rule
from opcodes import *

"""
Rule:
SHR(B, AND(X, A)) -> AND(SHR(B, X), A >> B)
SHR(B, AND(A, X)) -> AND(SHR(B, X), A >> B)
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
nonopt_1 = SHR(B, AND(X, A));
nonopt_2 = SHR(B, AND(A, X));

# Optimized result
Mask = SHR(B, A);
opt = AND(SHR(B, X), Mask);

rule.check(nonopt_1, opt)
rule.check(nonopt_2, opt)
