contract C {
	uint public x;

	function f() public {
		x = 2;
		x = 3;
		C c = this;
		uint y = c.x();
		assert(y == 3); // should hold
		assert(y == 2); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTTargets: assert
// ----
// Warning 6328: (138-152): CHC: Assertion violation happens here.
