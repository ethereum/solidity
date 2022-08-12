from opcodes import SIGNEXTEND
from rule import Rule
from util import BVSignedCleanupFunction, BVSignedUpCast
from z3 import BitVec, BitVecVal, Concat

"""
Overflow checked signed integer multiplication.
"""

n_bits = 256

# Check that YulUtilFunction::cleanupFunction cleanup matches BVSignedCleanupFunction
for type_bits in range(8,256,8):

	rule = Rule()

	# Input vars
	X = BitVec('X', n_bits)
	arg = BitVecVal(type_bits / 8 - 1, n_bits)

	cleaned_reference = BVSignedCleanupFunction(X, type_bits)
	cleaned = SIGNEXTEND(arg, X)

	rule.check(cleaned, cleaned_reference)


# Check that BVSignedCleanupFunction properly cleans up values.
for type_bits in range(8,256,8):

	rule = Rule()

	# Input vars
	X_short = BitVec('X', type_bits)
	dirt = BitVec('dirt', n_bits - type_bits)

	X = BVSignedUpCast(X_short, n_bits)
	X_dirty = Concat(dirt, X_short)
	X_cleaned = BVSignedCleanupFunction(X_dirty, type_bits)


	rule.check(X, X_cleaned)
