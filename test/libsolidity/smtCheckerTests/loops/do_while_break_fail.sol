pragma experimental SMTChecker;

contract C {
	function f() public pure {
		uint x;
		do {
			break;
			x = 1;
		} while (x == 0);
		assert(x == 1);
	}
}
// ====
// SMTSolvers: z3
// ----
// Warning 5740: (104-109): Unreachable code.
// Warning 5740: (122-128): Unreachable code.
// Warning 6328: (133-147): Assertion violation happens here
