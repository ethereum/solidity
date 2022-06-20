from opcodes import AND, SDIV, MUL, EQ, ISZERO, OR, SLT
from rule import Rule
from util import BVSignedUpCast, BVSignedMin, BVSignedCleanupFunction
from z3 import BVMulNoOverflow, BVMulNoUnderflow, BitVec, Not, Or

"""
Overflow checked signed integer multiplication.
"""

# Approximation with 16-bit base types.
n_bits = 12

for type_bits in [4, 6, 8, 12]:

	rule = Rule()

	# Input vars
	X_short = BitVec('X', type_bits)
	Y_short = BitVec('Y', type_bits)

	# Z3's overflow and underflow conditions
	actual_overflow = Not(BVMulNoOverflow(X_short, Y_short, True))
	actual_underflow = Not(BVMulNoUnderflow(X_short, Y_short))

	# cast to full n_bits values
	X = BVSignedUpCast(X_short, n_bits)
	Y = BVSignedUpCast(Y_short, n_bits)
	product_raw = MUL(X, Y)
	#remove any overflown bits
	product = BVSignedCleanupFunction(product_raw, type_bits)

	# Constants
	min_value = BVSignedMin(type_bits, n_bits)

	# Overflow and underflow checks in YulUtilFunction::overflowCheckedIntMulFunction
	if type_bits > n_bits / 2:
		sol_overflow_check_1 = ISZERO(OR(ISZERO(X), EQ(Y, SDIV(product, X))))
		if type_bits == n_bits:
			sol_overflow_check_2 = AND(SLT(X, 0), EQ(Y, min_value))
			sol_overflow_check = Or(sol_overflow_check_1 != 0, sol_overflow_check_2 != 0)
		else:
			sol_overflow_check = (sol_overflow_check_1 != 0)
	else:
		sol_overflow_check = (ISZERO(EQ(product, product_raw)) != 0)

	rule.check(Or(actual_overflow, actual_underflow), sol_overflow_check)
