from rule import Rule
from opcodes import *

"""
byte(A, shl(B, X))
given B % 8 == 0 && A <= 32 && B <= 256
->
byte(A + B / 8, X)
"""

rule = Rule()

n_bits = 256

# Input vars
X = BitVec('X', n_bits)
A = BitVec('A', n_bits)
B = BitVec('B', n_bits)

# Non optimized result
nonopt = BYTE(A, SHL(B, X))
# Optimized result
opt = BYTE(A + B / 8, X)

rule.require(B % 8 == 0)
rule.require(ULE(A, 32))
rule.require(ULE(B, 256))

rule.check(nonopt, opt)
