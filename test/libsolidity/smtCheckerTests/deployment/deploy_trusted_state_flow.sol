contract D {
	uint x;
	function inc() public { ++x; }
	function f() public view returns (uint) { return x; }
}

contract C {
	D d;
	constructor() {
		d = new D();
		assert(d.f() == 0); // should hold
	}
	function g() public view {
		assert(d.f() == 0); // should fail
	}
}
// ====
// SMTEngine: all
// SMTExtCalls: trusted
// SMTIgnoreOS: macos
// ----
// Warning 1218: (233-251): CHC: Error trying to invoke SMT solver.
// Warning 4984: (47-50): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 6328: (233-251): CHC: Assertion violation might happen here.
// Warning 2661: (47-50): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 4661: (233-251): BMC: Assertion violation happens here.
