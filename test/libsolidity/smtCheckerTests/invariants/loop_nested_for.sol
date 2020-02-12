pragma experimental SMTChecker;

contract Simple {
	function f() public pure {
		uint x;
		uint y;
		for (x = 10; y < x; ++y)
		{
			for (x = 0; x < 10; ++x) {}
			assert(x == 10);
		}
		assert(y == x);
	}
}
// ====
// SMTSolvers: z3
// ----
// Warning: (164-179): Error trying to invoke SMT solver.
// Warning: (187-201): Error trying to invoke SMT solver.
// Warning: (164-179): Assertion violation happens here
// Warning: (187-201): Assertion violation happens here
