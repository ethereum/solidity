pragma experimental SMTChecker;

contract C {
	function f() pure public {
		require(false);
		// This is not reachable.
		assert(false);
	}

	function g() pure public {
		revert();
		// This is not reachable.
		assert(false);
	}
}
// ----
// Warning 5740: (211-224): Unreachable code.
// Warning 6328: (211-224): Assertion violation happens here.
// Warning 4588: (171-179): Assertion checker does not yet implement this type of function call.
