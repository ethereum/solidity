from z3 import BitVec
from rule import Rule
from opcodes import SMOD


n_bits = 256
X = BitVec('X', n_bits)
Y = BitVec('Y', n_bits)


def smod_test(a, b, r, op=SMOD):
    r"""
    Tests SMOD with predefined parameters a,b against an expected result r.
    """
    to_check = Rule()
    to_check.require(X == a)
    to_check.require(Y == b)
    to_check.check(op(X, Y), r)


# tests from:
# https://github.com/ethereum/tests/blob/2e37a9f41167534b07e0e8f247ea934a5fe3cac9/src/GeneralStateTestsFiller/VMTests/vmArithmeticTest/smodFiller.yml
smod_test(2, 3, 2)
smod_test(-1, 2, -1)
smod_test(0, -1, 0)
smod_test(3, 0, 0)
smod_test(-2, 3, -2)
smod_test(-2, 3, -2)
smod_test(16, 0, -1, op=lambda x, y: SMOD(x, y) - 1)

# more tests
rule = Rule()
rule.require(X == 7)
rule.require(Y == 5)
rule.check(SMOD(X, Y), 2)

rule = Rule()
rule.require(X == 7)
rule.require(Y == 5)
rule.check(SMOD(X, Y), 2)

rule = Rule()
rule.require(X == -7)
rule.require(Y == 5)
rule.check(SMOD(X, Y), -2)

rule = Rule()
rule.require(X == 7)
rule.require(Y == -5)
rule.check(SMOD(X, Y), 2)

rule = Rule()
rule.require(X == -7)
rule.require(Y == -5)
rule.check(SMOD(X, Y), -2)

for k in [-7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7]:
    for i in [-13, -12, -11, 10, -7, -2, 1, 0, 1, 2, 7, 10, 11, 12, 13]:
        rule = Rule()
        rule.require(X == i)
        rule.require(Y == k)
        if k != 0:
            rule.check(SMOD(X, Y) % Y, X % Y)
        else:
            rule.check(SMOD(X, Y), 0)
