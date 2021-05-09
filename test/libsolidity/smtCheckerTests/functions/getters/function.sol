contract C {
	function () external returns (uint) public g;

	function f() public {
		g = this.X;
		function () external returns (uint) e = this.g();
		assert(e() == g()); // should hold, but fails because of the lack of support for tracking function pointers
		assert(e() == 1); // should fail
	}

	function X() public pure returns (uint) {
		return 42;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (152-170): CHC: Assertion violation happens here.\nCounterexample:\ng = 0\ne = 0\n\nTransaction trace:\nC.constructor()\nState: g = 0\nC.f()
// Warning 6328: (262-278): CHC: Assertion violation happens here.\nCounterexample:\ng = 0\ne = 0\n\nTransaction trace:\nC.constructor()\nState: g = 0\nC.f()
