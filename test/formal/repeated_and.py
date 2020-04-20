from rule import Rule
from opcodes import *

"""
Rule:
AND(AND(X, Y), Y) -> AND(X, Y)
AND(Y, AND(X, Y)) -> AND(X, Y)
AND(AND(Y, X), Y) -> AND(Y, X)
AND(Y, AND(Y, X)) -> AND(Y, X)
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
nonopt_1 = AND(AND(X, Y), Y)
nonopt_2 = AND(Y, AND(X, Y))
nonopt_3 = AND(AND(Y, X), Y)
nonopt_4 = AND(Y, AND(Y, X))

# Optimized result
opt_1 = AND(X, Y)
opt_2 = AND(Y, X)

rule.check(nonopt_1, opt_1)
rule.check(nonopt_2, opt_1)
rule.check(nonopt_3, opt_2)
rule.check(nonopt_4, opt_2)
