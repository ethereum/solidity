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
// SMTEngine: all
// SMTSolvers: z3
// ----
// Warning 5740: (71-76='x = 1'): Unreachable code.
// Warning 5740: (89-95='x == 0'): Unreachable code.
// Warning 6328: (100-114='assert(x == 1)'): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\n\nTransaction trace:\nC.constructor()\nC.f()
