from rule import Rule
from opcodes import *
from util import *

"""
Overflow checked unsigned integer multiplication.
"""

# Approximation with 16-bit base types.
n_bits = 16
type_bits = 8

while type_bits <= n_bits:

	rule = Rule()

	# Input vars
	X_short = BitVec('X', type_bits)
	Y_short = BitVec('Y', type_bits)

	# Z3's overflow condition
	actual_overflow = Not(BVMulNoOverflow(X_short, Y_short, False))

	# cast to full n_bits values
	X = BVUnsignedUpCast(X_short, n_bits)
	Y = BVUnsignedUpCast(Y_short, n_bits)

	# Constants
	maxValue = BVUnsignedMax(type_bits, n_bits)

	# Overflow check in YulUtilFunction::overflowCheckedIntMulFunction
	overflow_check = AND(ISZERO(ISZERO(X)), GT(Y, DIV(maxValue, X)))

	rule.check(overflow_check != 0, actual_overflow)

	type_bits *= 2
