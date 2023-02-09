contract C {
	uint public x;

	function f() public {
		x = 2;
		x = 3;
		uint y = this.x();
		assert(y == 3); // should hold
		assert(y == 2); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTIgnoreCex: yes
// SMTTargets: assert
// ----
// Warning 6328: (127-141): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
