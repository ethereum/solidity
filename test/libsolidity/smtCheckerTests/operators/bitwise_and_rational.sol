pragma experimental SMTChecker;

contract C {
	function f() public pure {
		assert(1 & 0 != 0);
		assert(-1 & 3 == 3);
		assert(-1 & -1 == -1);
		assert(-1 & 127 == 127);
	}
}
// ----
// Warning 6328: (76-94): Assertion violation happens here
