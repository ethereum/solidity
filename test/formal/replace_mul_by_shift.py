from opcodes import DIV, MUL, SHL, SHR
from rule import Rule
from z3 import BitVec

"""
Rule:
MUL(X, A) -> SHL(k, X)
MUL(A, X) -> SHL(k, X)
DIV(X, A) -> SHR(k, X)
Requirements:
A == 1 << K
"""

rule = Rule()

n_bits = 32

# Input vars
X = BitVec('X', n_bits)
A = BitVec('A', n_bits)
K = BitVec('K', n_bits)

# Requirements
rule.require(A == SHL(K, 1))

# Non optimized result
nonopt_1 = MUL(X, A)
nonopt_2 = MUL(A, X)
nonopt_3 = DIV(X, A)

# Optimized result
opt_1 = SHL(K, X)
opt_2 = SHL(K, X)
opt_3 = SHR(K, X)

rule.check(nonopt_1, opt_1)
rule.check(nonopt_2, opt_2)
rule.check(nonopt_3, opt_3)
