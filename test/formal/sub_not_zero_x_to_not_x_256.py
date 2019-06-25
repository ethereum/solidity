from rule import Rule
from opcodes import *

"""
Rule:
SUB(~0, X) -> NOT(X)
Requirements:
"""

rule = Rule()

n_bits = 256

# Input vars
X = BitVec('X', n_bits)

# Constants
ZERO = BitVecVal(0, n_bits)

# Non optimized result
nonopt = SUB(~ZERO, X)

# Optimized result
opt = NOT(X)

rule.check(nonopt, opt)
