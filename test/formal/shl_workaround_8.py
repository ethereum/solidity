from rule import Rule
from opcodes import *

"""
Shift left workaround that Solidity implements
due to a bug in Boost.
"""

rule = Rule()

n_bits = 8
bigint_bits = 16

# Input vars
X = BitVec('X', n_bits)
A = BitVec('A', n_bits)
B = BitVec('B', bigint_bits)

# Compute workaround
workaround = Int2BV(
	BV2Int(
		(Int2BV(BV2Int(X), bigint_bits) << Int2BV(BV2Int(A), bigint_bits)) &
		Int2BV(BV2Int(Int2BV(IntVal(-1), n_bits)), bigint_bits)
	), n_bits
)

rule.check(workaround, SHL(A, X))
