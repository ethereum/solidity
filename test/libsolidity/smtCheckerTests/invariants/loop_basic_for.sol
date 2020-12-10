pragma experimental SMTChecker;

contract Simple {
	function f(uint x) public pure {
		uint y;
		for (y = 0; y < x; ++y) {}
		assert(y == x);
	}
}
// ====
// SMTSolvers: z3
// ----
// Warning 4984: (116-119): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
