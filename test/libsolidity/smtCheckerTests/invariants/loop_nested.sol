pragma experimental SMTChecker;

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
// SMTSolvers: z3
// ----
// Warning: (172-187): Error trying to invoke SMT solver.
// Warning: (195-209): Error trying to invoke SMT solver.
// Warning: (172-187): Assertion violation happens here
// Warning: (195-209): Assertion violation happens here
