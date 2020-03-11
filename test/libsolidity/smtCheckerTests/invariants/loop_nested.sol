pragma experimental SMTChecker;

// This test gets different results on Linux and OSX.
// Re-enable when fixed (SMTSolvers: z3)

contract Simple {
	function f() public pure {
		uint x = 10;
		uint y;
		while (y < x)
		{
			++y;
			x = 0;
			while (x < 10)
				++x;
			assert(x == 10);
		}
		assert(y == x);
	}
}
// ====
// SMTSolvers: none
// ----
// Warning: (195-209): Error trying to invoke SMT solver.
// Warning: (195-209): Assertion violation happens here
