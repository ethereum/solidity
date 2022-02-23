contract C {
	function f() public pure {
		uint a = 0;
		while (true) {
			do {
				break;
				a = 2;
			} while (true);
			a = 1;
			break;
		}
		assert(a == 2);
	}
}
// ====
// SMTEngine: all
// SMTSolvers: z3
// ----
// Warning 5740: (95-100): Unreachable code.
// Warning 5740: (114-118): Unreachable code.
// Warning 6328: (147-161): CHC: Assertion violation happens here.\nCounterexample:\n\na = 1\n\nTransaction trace:\nC.constructor()\nC.f()
