from opcodes import ISZERO, DIV, MUL, EQ, OR
from rule import Rule
from util import BVUnsignedUpCast, BVUnsignedCleanupFunction
from z3 import BitVec, Not, BVMulNoOverflow

"""
Overflow checked unsigned integer multiplication.
"""

# Approximation with 16-bit base types.
n_bits = 12

for type_bits in [4, 6, 8, 12]:

	rule = Rule()

	# Input vars
	X_short = BitVec('X', type_bits)
	Y_short = BitVec('Y', type_bits)

	# Z3's overflow condition
	actual_overflow = Not(BVMulNoOverflow(X_short, Y_short, False))

	# cast to full n_bits values
	X = BVUnsignedUpCast(X_short, n_bits)
	Y = BVUnsignedUpCast(Y_short, n_bits)
	product_raw = MUL(X, Y)
	#remove any overflown bits
	product = BVUnsignedCleanupFunction(product_raw, type_bits)

	# Overflow check in YulUtilFunction::overflowCheckedIntMulFunctions
	if type_bits > n_bits / 2:
		overflow_check = ISZERO(OR(ISZERO(X), EQ(Y, DIV(product, X))))
	else:
		overflow_check = ISZERO(EQ(product, product_raw))

	rule.check(overflow_check != 0, actual_overflow)
