from opcodes import ADD, SUB
from rule import Rule
from z3 import BitVec

"""
Rules:
SUB(SUB(X, A), Y) -> SUB(SUB(X, Y), A)
SUB(SUB(A, X), Y) -> SUB(A, ADD(X, Y))
SUB(X, SUB(Y, A)) -> ADD(SUB(X, Y), A)
SUB(X, SUB(A, Y)) -> ADD(ADD(X, Y), -A)
"""

rule = Rule()

n_bits = 256

# Input vars
X = BitVec('X', n_bits)
Y = BitVec('Y', n_bits)
A = BitVec('A', n_bits)

rule.check(
    SUB(SUB(X, A), Y),
    SUB(SUB(X, Y), A)
)
rule.check(
    SUB(SUB(A, X), Y),
    SUB(A, ADD(X, Y))
)
rule.check(
    SUB(X, SUB(Y, A)),
    ADD(SUB(X, Y), A)
)
rule.check(
    SUB(X, SUB(A, Y)),
    ADD(ADD(X, Y), SUB(0, A))
)
