contract D {
	bool b;
	function s() public { b = true; }
	function f() public view returns (bool) { return b; }
}

contract C {
	D d;
	constructor() {
		d = new D();
	}
	function g() public view {
		assert(d.f()); // should fail
	}
}
// ====
// SMTEngine: all
// SMTExtCalls: trusted
// ----
// Warning 6328: (199-212): CHC: Assertion violation happens here.\nCounterexample:\nd = 0\n\nTransaction trace:\nC.constructor()\nState: d = 0\nC.g()\n    D.f() -- trusted external call
