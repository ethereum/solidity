from opcodes import BYTE
from rule import Rule
from z3 import BitVec

"""
byte(A, X) -> 0
given A >= WordSize / 8
"""

rule = Rule()

n_bits = 256

# Input vars
X = BitVec('X', n_bits)
A = BitVec('A', n_bits)

rule.require(A >= n_bits / 8)
rule.check(BYTE(A, X), 0)
