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
// Warning 8364: (140-146): Assertion checker does not yet implement type function () view external returns (function () external returns (uint256))
// Warning 6328: (152-170): CHC: Assertion violation happens here.
// Warning 6328: (262-278): CHC: Assertion violation happens here.
