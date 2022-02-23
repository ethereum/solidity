contract C
{
	function f(uint x) public pure {
		for (x = 0; x < 10; ++x)
			break;
		assert(x == 0);
	}
}
// ====
// SMTEngine: all
// SMTSolvers: z3
// ----
// Warning 5740: (69-72): Unreachable code.
