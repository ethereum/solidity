contract C {
	uint[2] a;
	function f() public view {
		assert(a.length == 2); // should hold
		assert(a.length < 2); // should fail
		assert(a.length > 2); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (95-115): CHC: Assertion violation happens here.\nCounterexample:\na = [0, 0]\n\nTransaction trace:\nC.constructor()\nState: a = [0, 0]\nC.f()
// Warning 6328: (134-154): CHC: Assertion violation happens here.\nCounterexample:\na = [0, 0]\n\nTransaction trace:\nC.constructor()\nState: a = [0, 0]\nC.f()
// Info 1180: Contract invariant(s) for :C:\n(!(a.length <= 1) && !(a.length >= 3))\n
