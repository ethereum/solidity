pragma experimental SMTChecker;

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
// SMTSolvers: z3
// ----
// Warning 5740: (128-131): Unreachable code.
