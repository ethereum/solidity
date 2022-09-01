from z3 import BitVecVal, Concat, If

def BVUnsignedUpCast(x, n_bits):
	assert x.size() <= n_bits
	if x.size() < n_bits:
		return Concat(BitVecVal(0, n_bits - x.size()), x)
	else:
		return x

def BVUnsignedMax(type_bits, n_bits):
	assert type_bits <= n_bits
	return BitVecVal((1 << type_bits) - 1, n_bits)

def BVSignedUpCast(x, n_bits):
	assert x.size() <= n_bits
	if x.size() < n_bits:
		return Concat(If(x < 0, BitVecVal(-1, n_bits - x.size()), BitVecVal(0, n_bits - x.size())), x)
	else:
		return x

def BVSignedMax(type_bits, n_bits):
	assert type_bits <= n_bits
	return BitVecVal((1 << (type_bits - 1)) - 1, n_bits)

def BVSignedMin(type_bits, n_bits):
	assert type_bits <= n_bits
	return BitVecVal(-(1 << (type_bits - 1)), n_bits)

def BVSignedCleanupFunction(x, type_bits):
	assert x.size() >= type_bits
	sign_mask = BitVecVal(1, x.size()) << (type_bits - 1)
	bit_mask = (BitVecVal(1, x.size()) << type_bits) - 1
	return If(
		x & sign_mask == 0,
		x & bit_mask,
		x | ~bit_mask
	)

def BVUnsignedCleanupFunction(x, type_bits):
	assert x.size() >= type_bits
	bit_mask = (BitVecVal(1, x.size()) << type_bits) - 1
	return x & bit_mask
