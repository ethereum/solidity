pragma experimental SMTChecker;

contract C {
	function f() internal pure returns (bool) {
		bool b;
		assembly { b := 1 } // This assignment is overapproximated at the moment, we don't know value of b after the assembly block
		return b;
	}
	function g() public pure {
		assert(f()); // False positive currently
		assert(!f()); // should fail, now because of overapproximation in the analysis
		require(f()); // BMC constant value not detected at the moment
		require(!f()); // BMC constant value not ddetected at the moment
	}
}
// ----
// Warning 7737: (103-122): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
// Warning 6328: (272-283): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()\n    C.f() -- internal call
// Warning 6328: (315-327): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()\n    C.f() -- internal call\n    C.f() -- internal call
// Warning 7737: (103-122): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
// Warning 7737: (103-122): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
// Warning 7737: (103-122): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
// Warning 7737: (103-122): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
// Warning 7737: (103-122): Inline assembly may cause SMTChecker to produce spurious warnings (false positives).
