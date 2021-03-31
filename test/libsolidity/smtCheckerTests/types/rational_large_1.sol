contract c {
	function f() public pure returns (uint) {
		uint x = 8e130%9;
		assert(x == 8);
		assert(x != 8);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6321: (48-52): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6328: (96-110): CHC: Assertion violation happens here.\nCounterexample:\n\n = 0\nx = 8\n\nTransaction trace:\nc.constructor()\nc.f()
