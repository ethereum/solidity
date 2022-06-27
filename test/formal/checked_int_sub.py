from opcodes import AND, ISZERO, SLT, SGT, SUB
from rule import Rule
from util import BVSignedMax, BVSignedMin, BVSignedUpCast
from z3 import BitVec, BVSubNoOverflow, BVSubNoUnderflow, Not

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
	diff = SUB(X, Y)

	# Constants
	maxValue = BVSignedMax(type_bits, n_bits)
	minValue = BVSignedMin(type_bits, n_bits)

	# Overflow and underflow checks in YulUtilFunction::overflowCheckedIntSubFunction
	if type_bits == 256:
		underflow_check = AND(ISZERO(SLT(Y, 0)), SGT(diff, X))
		overflow_check = AND(SLT(Y, 0), SLT(diff, X))
	else:
		underflow_check = SLT(diff, minValue)
		overflow_check = SGT(diff, maxValue)

	type_bits += 8

	rule.check(actual_underflow, underflow_check != 0)
	rule.check(actual_overflow, overflow_check != 0)
