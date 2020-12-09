pragma experimental SMTChecker;
contract C {
	function f() public pure returns(int) {
		int a;
		((,, a)) = ((((((1, 3, (((((2)))))))))));
		assert(a == 2);
		assert(a == 3);
	}
}
// ----
// Warning 6321: (79-82): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6328: (159-173): CHC: Assertion violation happens here.\nCounterexample:\n\n\n = 0\n\nTransaction trace:\nconstructor()\nf()
