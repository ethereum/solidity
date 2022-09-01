from opcodes import MOD, MUL, MULMOD
from rule import Rule
from z3 import BitVec

"""
Rule:
MOD(MUL(X, Y), A) -> MULMOD(X, Y, A)
given
    A > 0
    A & (A - 1) == 0
"""

rule = Rule()

n_bits = 8

# Input vars
X = BitVec('X', n_bits)
Y = BitVec('Y', n_bits)
A = BitVec('A', n_bits)

# Non optimized result
nonopt = MOD(MUL(X, Y), A)

# Optimized result
opt = MULMOD(X, Y, A)

rule.require(A > 0)
rule.require(((A & (A - 1)) == 0))

rule.check(nonopt, opt)
