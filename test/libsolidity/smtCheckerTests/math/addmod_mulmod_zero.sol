contract C {
	function f(uint256 d) public pure {
		uint x = addmod(1, 2, d);
		assert(x < d);
	}

	function g(uint256 d) public pure {
		uint x = mulmod(1, 2, d);
		assert(x < d);
	}

	function h() public pure returns (uint256) {
		uint x = mulmod(0, 1, 2);
		uint y = mulmod(1, 0, 2);
		assert(x == y);
		uint z = addmod(0, 1, 2);
		uint t = addmod(1, 0, 2);
		assert(z == t);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6321: (220-227): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 4281: (61-76): CHC: Division by zero happens here.\nCounterexample:\n\nd = 0\nx = 0\n\nTransaction trace:\nC.constructor()\nC.f(0)
// Warning 6328: (80-93): CHC: Assertion violation happens here.\nCounterexample:\n\nd = 0\nx = 0\n\nTransaction trace:\nC.constructor()\nC.f(0)
// Warning 4281: (147-162): CHC: Division by zero happens here.\nCounterexample:\n\nd = 0\nx = 0\n\nTransaction trace:\nC.constructor()\nC.g(0)
// Warning 6328: (166-179): CHC: Assertion violation happens here.\nCounterexample:\n\nd = 0\nx = 0\n\nTransaction trace:\nC.constructor()\nC.g(0)
