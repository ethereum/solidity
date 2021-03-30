pragma experimental SMTChecker;

contract C {
	function f() public pure returns (bool) {
		bool b;
		int x = 42;
		assembly { b := 1 }
		assert(x == 42); // should hold
		assert(b); // should hold, but fails due to overapproximation
		return b;
	}
}
// ----
// Warning 7737: (115-134): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
// Warning 6328: (171-180): CHC: Assertion violation happens here.\nCounterexample:\n\n = false\nb = false\nx = 42\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 7737: (115-134): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
