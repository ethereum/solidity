contract C {
	function f() public pure returns(int) {
		int a;
		(,, a) = ((((((1, 3, (((((2)))))))))));
		assert(a == 2);
		assert(a == 3);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6321: (47-50): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6328: (125-139): CHC: Assertion violation happens here.\nCounterexample:\n\n = 0\na = 2\n\nTransaction trace:\nC.constructor()\nC.f()
