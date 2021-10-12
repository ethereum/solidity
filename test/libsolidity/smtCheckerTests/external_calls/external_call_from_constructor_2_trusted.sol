contract C {
	uint z = this.g(2);

	function g(uint _x) public pure returns (uint) {
		assert(_x > 0); // should fail
		return _x;
	}

	function f() public view {
		assert(z == 2); // should hold
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (87-101): CHC: Assertion violation happens here.\nCounterexample:\nz = 2\n_x = 0\n = 0\n\nTransaction trace:\nC.constructor()\nState: z = 2\nC.g(0)
// Info 1180: Contract invariant(s) for :C:\n(!(z >= 3) && !(z <= 1))\n
