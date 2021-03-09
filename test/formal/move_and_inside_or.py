from rule import Rule
from opcodes import *

"""
Rule:
AND(OR(AND(X, A), Y), B) -> OR(AND(X, A & B), AND(Y, B))
"""

rule = Rule()

# bit width is irrelevant
n_bits = 128

# Input vars
X = BitVec('X', n_bits)
Y = BitVec('Y', n_bits)
A = BitVec('A', n_bits)
B = BitVec('B', n_bits)

# Non optimized result, explicit form
nonopt = AND(OR(AND(X, A), Y), B)

# Optimized result
opt = OR(AND(X, A & B), AND(Y, B))

rule.check(nonopt, opt)

# Now the forms as they are constructod in the code.
for inner in [AND(X, A), AND(A, X)]:
    for second in [OR(inner, Y), OR(Y, inner)]:
        rule.check(AND(second, B), opt)
        rule.check(AND(B, second), opt)
