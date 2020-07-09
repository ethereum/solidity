from rule import Rule
from opcodes import *

"""
byte(A, shr(B, X))
given B % 8 == 0 && A < n_bits/8 && B <= n_bits && A < B / 8
->
0
"""

rule = Rule()

n_bits = 256

# Input vars
X = BitVec('X', n_bits)
A = BitVec('A', n_bits)
B = BitVec('B', n_bits)

# Non optimized result
nonopt = BYTE(A, SHR(B, X))
# Optimized result
opt = 0

rule.require(B % 8 == 0)
rule.require(ULT(A, n_bits/8))
rule.require(ULE(B, n_bits))
rule.require(ULT(A, DIV(B,8)))

rule.check(nonopt, opt)
