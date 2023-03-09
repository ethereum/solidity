contract A {
	int x = 0;

	modifier m virtual {
		assert(x == 0); // should hold
		assert(x == 42); // should fail
		_;
	}
}
contract C is A {

	modifier m override {
		assert(x == 1); // This assert is not reachable, should NOT be reported
		_;
	}

	function f() public A.m returns (uint) {
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (83-98): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
