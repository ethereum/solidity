from opcodes import AND
from rule import Rule
from util import BVUnsignedCleanupFunction, BVUnsignedUpCast
from z3 import BitVec, BitVecVal, Concat

"""
Overflow checked unsigned integer multiplication.
"""

n_bits = 256

# Check that YulUtilFunction::cleanupFunction cleanup matches BVUnsignedCleanupFunction
for type_bits in range(8,256,8):

	rule = Rule()

	# Input vars
	X = BitVec('X', n_bits)
	mask = BitVecVal((1 << type_bits) - 1, n_bits)

	cleaned_reference = BVUnsignedCleanupFunction(X, type_bits)
	cleaned = AND(X, mask)

	rule.check(cleaned, cleaned_reference)

# Check that BVUnsignedCleanupFunction properly cleans up values.
for type_bits in range(8,256,8):

	rule = Rule()

	# Input vars
	X_short = BitVec('X', type_bits)
	dirt = BitVec('dirt', n_bits - type_bits)

	X = BVUnsignedUpCast(X_short, n_bits)
	X_dirty = Concat(dirt, X_short)
	X_cleaned = BVUnsignedCleanupFunction(X_dirty, type_bits)


	rule.check(X, X_cleaned)
