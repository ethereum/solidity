abstract contract A {
	uint s;

	function f() public mod(s) {}
	modifier mod(uint x) virtual;
}

contract B is A {
	modifier mod(uint x) override {
		require(x == 42);
		_;
		assert(x == 42); // should hold
		assert(x == 0); // should fail
	}

	function set(uint x) public {
		s = x;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (209-223): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
