from rule import Rule
from opcodes import *
from util import *

"""
Overflow checked signed integer addition.
"""

n_bits = 256
type_bits = 8

while type_bits <= n_bits:

	rule = Rule()

	# Input vars
	X_short = BitVec('X', type_bits)
	Y_short = BitVec('Y', type_bits)

	# Z3's overflow and underflow conditions
	actual_overflow = Not(BVAddNoOverflow(X_short, Y_short, True))
	actual_underflow = Not(BVAddNoUnderflow(X_short, Y_short))

	# cast to full n_bits values
	X = BVSignedUpCast(X_short, n_bits)
	Y = BVSignedUpCast(Y_short, n_bits)

	# Constants
	maxValue = BVSignedMax(type_bits, n_bits)
	minValue = BVSignedMin(type_bits, n_bits)

	# Overflow and underflow checks in YulUtilFunction::overflowCheckedIntAddFunction
	overflow_check = AND(ISZERO(SLT(X, 0)), SGT(Y, SUB(maxValue, X)))
	underflow_check = AND(SLT(X, 0), SLT(Y, SUB(minValue, X)))

	rule.check(actual_overflow, overflow_check != 0)
	rule.check(actual_underflow, underflow_check != 0)

	type_bits *= 2
