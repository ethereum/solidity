import sys
from z3 import Solver, Int, unsat

"""
Tests that the conditions inside RedundantStoreEliminator::knownUnrelated
only return "unrelated" incorrectly if one of the operation reverts
due to large memory access.
"""

n_bits = 256

solver = Solver()
solver.set("timeout", 60000)

def restrict(x):
    solver.add(x >= 0)
    solver.add(x < 2**n_bits)

def restrictedInt(x):
    var = Int(x)
    restrict(var)
    return var

start1 = restrictedInt('start1')
length1 = restrictedInt('length1')
start2 = restrictedInt('start2')
length2 = restrictedInt('length2')

k = Int('k')
diff = Int('diff')
solver.add(diff == start2 - start1 + k * 2**n_bits)
restrict(diff)
# diff is the result of sub(start2, start1) in EVM

# These are the conditions in the code.
solver.add(diff >= length1)
solver.add(diff <= 2**(n_bits-1))

# We check that the two conditions are conflicting:
# - overlap
# - start1 is small

# Overlap:
# x is a potential point where the memory operations
# overlap.
# Note that we do not use wrapping arithmetic
# here, because it is not done in the EVM either.
# For example calldatacopy(2**256 - 2, 0, 10)
# (copy 10 bytes from calldata position zero to memory
# position 2**256 - 2) would not write to memory position
# zero either.
x = Int('x')
solver.add(start1 <= x)
solver.add(x < start1 + length1)
solver.add(start2 <= x)
solver.add(x < start2 + length2)

# start1 is "small":
solver.add(start1 < 2**(n_bits-1))

if solver.check() != unsat:
  print("Expected unsat but got something else")
  sys.exit(1)
