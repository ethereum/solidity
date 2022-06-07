from opcodes import AND, SDIV, SGT, SLT, MUL, ISZERO, XOR, EQ
from rule import Rule
from util import BVSignedMax, BVSignedMin, BVSignedUpCast
from z3 import BVMulNoOverflow, BVMulNoUnderflow, BitVec, Not

"""
Overflow checked signed integer multiplication.
"""

# Approximation with 16-bit base types.
n_bits = 16
type_bits = 8

while type_bits <= n_bits:

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
	product = MUL(X, Y)

	# Constants
	maxValue = BVSignedMax(type_bits, n_bits)
	minValue = BVSignedMin(type_bits, n_bits)
	bitMask =  2**(type_bits-1)

	# Overflow and underflow checks in YulUtilFunction::overflowCheckedIntMulFunction
	if type_bits == n_bits:
		overflow_check = AND(ISZERO(ISZERO(Y)), ISZERO(EQ(X, SDIV(product, Y))))
		underflow_check = overflow_check
	else:
		overflow_check = AND(ISZERO(AND(XOR(X, Y),bitMask)), SGT(product, maxValue))
		underflow_check = AND(EQ(AND(XOR(X, Y), bitMask), bitMask), SLT(product, minValue))

	rule.check(actual_overflow, overflow_check != 0)
	rule.check(actual_underflow, underflow_check != 0)

	type_bits += 4
