contract C {
	uint x;
	function i() public { ++x; }
	function f() public {
		x = 0;
		C c = this;
		c.i();
		assert(x == 1); // should hold in trusted mode
		assert(x != 1); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTTargets: assert
// ----
// Warning 6328: (158-172): CHC: Assertion violation happens here.
