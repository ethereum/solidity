pragma experimental SMTChecker;

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
// ----
// Warning 6321: (253-260): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 4281: (94-109): CHC: Division by zero happens here.\nCounterexample:\n\nd = 0\nx = 0\n\nTransaction trace:\nC.constructor()\nC.f(0)
// Warning 6328: (113-126): CHC: Assertion violation happens here.\nCounterexample:\n\nd = 0\nx = 0\n\nTransaction trace:\nC.constructor()\nC.f(0)
// Warning 4281: (180-195): CHC: Division by zero happens here.\nCounterexample:\n\nd = 0\nx = 0\n\nTransaction trace:\nC.constructor()\nC.g(0)
// Warning 6328: (199-212): CHC: Assertion violation happens here.\nCounterexample:\n\nd = 0\nx = 0\n\nTransaction trace:\nC.constructor()\nC.g(0)
