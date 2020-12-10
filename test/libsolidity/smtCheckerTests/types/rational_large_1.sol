pragma experimental SMTChecker;
contract c {
	function f() public pure returns (uint) {
		uint x = 8e130%9;
		assert(x == 8);
		assert(x != 8);
	}
}
// ----
// Warning 6321: (80-84): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6328: (128-142): CHC: Assertion violation happens here.\nCounterexample:\n\n\n = 0\n\nTransaction trace:\nconstructor()\nf()
