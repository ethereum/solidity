from rule import Rule
from opcodes import *

"""
Rule:
OR(OR(X, Y), Y) -> OR(X, Y)
OR(Y, OR(X, Y)) -> OR(X, Y)
OR(OR(Y, X), Y) -> OR(Y, X)
OR(Y, OR(Y, X)) -> OR(Y, X)
Requirements:
"""

rule = Rule()

n_bits = 256

# Input vars
X = BitVec('X', n_bits)
Y = BitVec('Y', n_bits)

# Constants
BitWidth = BitVecVal(n_bits, n_bits)

# Requirements

# Non optimized result
nonopt_1 = OR(OR(X, Y), Y)
nonopt_2 = OR(Y, OR(X, Y))
nonopt_3 = OR(OR(Y, X), Y)
nonopt_4 = OR(Y, OR(Y, X))

# Optimized result
opt_1 = OR(X, Y)
opt_2 = OR(Y, X)

rule.check(nonopt_1, opt_1)
rule.check(nonopt_2, opt_1)
rule.check(nonopt_3, opt_2)
rule.check(nonopt_4, opt_2)
