contract D {
	uint x;
	function inc() public { ++x; }
	function f() public view returns (uint) { return x; }
}

contract C {
	function f() public {
		D d = new D();
		assert(d.f() == 0); // should hold
		d.inc();
		assert(d.f() == 1); // should hold
		d = new D();
		assert(d.f() == 0); // should hold
		assert(d.f() == 1); // should fail
	}
}
// ====
// SMTEngine: all
// SMTExtCalls: trusted
// ----
// Warning 4984: (47-50): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 6328: (304-322): CHC: Assertion violation might happen here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
// Warning 2661: (47-50): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 4661: (304-322): BMC: Assertion violation happens here.
