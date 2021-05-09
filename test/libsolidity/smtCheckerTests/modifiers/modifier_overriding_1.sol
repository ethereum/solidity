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
// Warning 6328: (209-223): CHC: Assertion violation happens here.\nCounterexample:\ns = 42\nx = 42\n\nTransaction trace:\nB.constructor()\nState: s = 0\nB.set(42)\nState: s = 42\nA.f()
