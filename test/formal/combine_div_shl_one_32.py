from rule import Rule
from opcodes import *

"""
Rule:
DIV(X, SHL(Y, 1)) -> SHR(Y, X)
Requirements:
"""

rule = Rule()

n_bits = 32

# Input vars
X = BitVec('X', n_bits)
Y = BitVec('Y', n_bits)

# Constants
ONE = BitVecVal(1, n_bits)

# Non optimized result
nonopt = DIV(X, SHL(Y, ONE))

# Optimized result
opt = SHR(Y, X)

rule.check(nonopt, opt)
