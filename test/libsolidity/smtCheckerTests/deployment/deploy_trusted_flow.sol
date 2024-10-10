contract D {
	uint x;
	function inc() public { require(x < 5); ++x; }
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
// SMTTargets: assert
// ----
// Warning 6328: (320-338): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
