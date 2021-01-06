from rule import Rule
from opcodes import *

"""
Rule:
ISZERO(SUB(X, Y)) -> EQ(X, Y)
"""

rule = Rule()

n_bits = 256

# Input vars
X = BitVec('X', n_bits)
Y = BitVec('Y', n_bits)

# Non optimized result
nonopt = ISZERO(SUB(X, Y))

# Optimized result
opt = EQ(X, Y)

rule.check(nonopt, opt)
