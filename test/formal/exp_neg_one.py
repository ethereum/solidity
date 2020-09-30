from rule import Rule
from opcodes import *
from util import *

"""
Checking conversion of exp(-1, X) to sub(isZero(and(X, 1)), and(X, 1))
"""

rule = Rule()
n_bits = 256

X = BitVec('X', n_bits)

exp_neg_one = If(MOD(X, 2) == 0, BitVecVal(1, n_bits), BVUnsignedMax(n_bits, n_bits))

rule.check(SUB(ISZERO(AND(X, 1)), AND(X, 1)), exp_neg_one)
