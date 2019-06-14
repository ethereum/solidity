from rule import Rule
from opcodes import *
from util import *

"""
Overflow checked signed integer division.
"""

n_bits = 256
type_bits = 8

while type_bits <= n_bits:

	rule = Rule()

	# Input vars
	X_short = BitVec('X', type_bits)
	Y_short = BitVec('Y', type_bits)

	# Z3's overflow conditions
	actual_overflow = Not(BVSDivNoOverflow(X_short, Y_short))

	# cast to full n_bits values
	X = BVSignedUpCast(X_short, n_bits)
	Y = BVSignedUpCast(Y_short, n_bits)

	# Constants
	minValue = BVSignedMin(type_bits, n_bits)

	# Overflow check in YulUtilFunction::overflowCheckedIntDivFunction
	overflow_check = AND(EQ(X, minValue), EQ(Y, SUB(0, 1)))

	rule.check(actual_overflow, overflow_check != 0)

	type_bits *= 2
