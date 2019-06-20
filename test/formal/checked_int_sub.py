from rule import Rule
from opcodes import *
from util import *

"""
Overflow checked signed integer subtraction.
"""

n_bits = 256
type_bits = 8

while type_bits <= n_bits:

	rule = Rule()

	# Input vars
	X_short = BitVec('X', type_bits)
	Y_short = BitVec('Y', type_bits)

	# Z3's overflow and underflow conditions
	actual_overflow = Not(BVSubNoOverflow(X_short, Y_short))
	actual_underflow = Not(BVSubNoUnderflow(X_short, Y_short, True))

	# cast to full n_bits values
	X = BVSignedUpCast(X_short, n_bits)
	Y = BVSignedUpCast(Y_short, n_bits)

	# Constants
	maxValue = BVSignedMax(type_bits, n_bits)
	minValue = BVSignedMin(type_bits, n_bits)

	# Overflow and underflow checks in YulUtilFunction::overflowCheckedIntSubFunction
	underflow_check = AND(ISZERO(SLT(Y, 0)), SLT(X, ADD(minValue, Y)))
	overflow_check = AND(SLT(Y, 0), SGT(X, ADD(maxValue, Y)))

	rule.check(actual_underflow, underflow_check != 0)
	rule.check(actual_overflow, overflow_check != 0)

	type_bits *= 2
