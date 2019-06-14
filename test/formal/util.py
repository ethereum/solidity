from z3 import *

def BVUnsignedUpCast(x, n_bits):
	assert(x.size() <= n_bits)
	if x.size() < n_bits:
		return Concat(BitVecVal(0, n_bits - x.size()), x)
	else:
		return x

def BVUnsignedMax(type_bits, n_bits):
	assert(type_bits <= n_bits)
	return BitVecVal((1 << type_bits) - 1, n_bits)

def BVSignedUpCast(x, n_bits):
	assert(x.size() <= n_bits)
	if x.size() < n_bits:
		return Concat(If(x < 0, BitVecVal(-1, n_bits - x.size()), BitVecVal(0, n_bits - x.size())), x)
	else:
		return x

def BVSignedMax(type_bits, n_bits):
	assert(type_bits <= n_bits)
	return BitVecVal((1 << (type_bits - 1)) - 1, n_bits)

def BVSignedMin(type_bits, n_bits):
	assert(type_bits <= n_bits)
	return BitVecVal(-(1 << (type_bits - 1)), n_bits)
