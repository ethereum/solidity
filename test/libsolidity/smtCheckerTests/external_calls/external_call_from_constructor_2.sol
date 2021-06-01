contract C {
	uint z = this.g(2);

	function g(uint _x) public pure returns (uint) {
		assert(_x > 0);
		return _x;
	}

	function f() public view {
		assert(z == 2); // should fail since we don't trust s.f's code
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (87-101): CHC: Assertion violation happens here.\nCounterexample:\nz = 2\n_x = 0\n = 0\n\nTransaction trace:\nC.constructor()\nState: z = 2\nC.g(0)
