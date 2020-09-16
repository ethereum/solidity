from rule import Rule
from opcodes import *
from util import *

"""
Checking conversion of exp(2, X) to shl(X, 1)
"""

rule = Rule()
n_bits = 256

# Proof of exp(2, X) = shl(X, 1) by induction:
#
# Base case: X = 0, exp(2, 0) = 1 = 1 = shl(0, 1)
# Inductive step: assuming exp(2, X) = shl(X, 1) for X <= N
#                 to prove: exp(2, N + 1) = shl(N + 1, 1)
#
# Notice that exp(2, N + 1) = 2 * exp(2, N) mod 2**256
# since exp(2, N) = shl(N, 1), it is enough to show that
# 2 * shl(N, 1) mod 2**256 = shl(N + 1, 1)
#
# Also note that N + 1 < 2**256

N = BitVec('N', n_bits)
inductive_step = 2 * SHL(N, 1)

rule.check(
    inductive_step,
    If(
        N == 2**256 - 1,
        0,
        SHL(N + 1, 1)
    )
)
