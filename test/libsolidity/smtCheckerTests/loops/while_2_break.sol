contract C
{
	function f() public pure {
		uint x = 0;
		while (x == 0) {
			++x;
			break;
			++x;
		}
		assert(x == 1);
	}
}
// ====
// SMTEngine: all
// SMTSolvers: z3
// ----
// Warning 5740: (95-98): Unreachable code.
