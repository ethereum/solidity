from opcodes import SLT, SGT, SUB, XOR
from rule import Rule
from util import BVSignedMax, BVSignedMin, BVSignedUpCast
from z3 import BitVec, BVSubNoOverflow, BVSubNoUnderflow, Not, Or

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
		actual_overflow_or_underflow = Or(actual_overflow, actual_underflow)

		overflow_or_underflow_check = XOR(SLT(Y, 0), SLT(X, diff))

		rule.check(actual_overflow_or_underflow,
				   overflow_or_underflow_check != 0)

	else:
		underflow_check = SLT(diff, minValue)
		overflow_check = SGT(diff, maxValue)

		rule.check(actual_underflow, underflow_check != 0)
		rule.check(actual_overflow, overflow_check != 0)

	type_bits += 8
