pragma experimental SMTChecker;

contract C {
	function f() public pure {
		uint x;
		do {
			break;
			x = 1;
		} while (x == 0);
		assert(x == 0);
	}
}
// ====
// SMTSolvers: z3
// ----
// Warning 5740: (104-109): Unreachable code.
// Warning 5740: (122-128): Unreachable code.
