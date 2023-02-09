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
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
