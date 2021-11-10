contract D {
	uint x;
	function s(uint _x) public { x = _x; }
	function f() public view returns (uint) { return x; }
}

contract C {
	D d;
	constructor() {
		d = new D();
	}
	function g() public view {
		assert(d.f() == 0); // should fail
	}
}
// ====
// SMTContract: C
// SMTEngine: all
// SMTExtCalls: trusted
// ----
// Warning 1218: (204-222): CHC: Error trying to invoke SMT solver.
// Warning 6328: (204-222): CHC: Assertion violation might happen here.
// Warning 4661: (204-222): BMC: Assertion violation happens here.
