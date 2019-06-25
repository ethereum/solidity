from rule import Rule
from opcodes import *

"""
Rule:
MUL(X, SHL(Y, 1)) -> SHL(Y, X)
MUL(SHL(X, 1), Y) -> SHL(X, Y)
Requirements:
"""

rule = Rule()

n_bits = 64

# Input vars
X = BitVec('X', n_bits)
Y = BitVec('Y', n_bits)

# Requirements

# Non optimized result
nonopt_1 = MUL(X, SHL(Y, 1))
nonopt_2 = MUL(SHL(X, 1), Y)

# Optimized result
opt_1 = SHL(Y, X)
opt_2 = SHL(X, Y)

rule.check(nonopt_1, opt_1)
rule.check(nonopt_2, opt_2)
