from opcodes import AND, SHL
from rule import Rule
from z3 import BitVec

"""
Rule:
AND(SHL(Z,X), SHL(Z,Y)) -> SHL(Z, AND(X,Y))
"""

rule = Rule()

n_bits = 128

# Input vars
X = BitVec('X', n_bits)
Y = BitVec('Y', n_bits)
Z = BitVec('Z', n_bits)

# Non optimized result
nonopt = AND(SHL(Z,X), SHL(Z,Y))

# Optimized result
opt = SHL(Z, AND(X,Y))

rule.check(nonopt, opt)
