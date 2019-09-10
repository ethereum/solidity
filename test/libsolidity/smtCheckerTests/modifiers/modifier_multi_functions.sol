pragma experimental SMTChecker;

contract C
{
	modifier m(uint a, uint b) {
		require(g(a, b));
		_;
	}

	modifier notZero(uint x) {
		require(x > 0);
		_;
	}

	function g(uint a, uint b) notZero(a) internal pure returns (bool) {
		return a > b;
	}

	function f(uint x) m(x, 0) public pure {
		assert(x > 0);
		assert(x > 1);
	}
}
// ----
// Warning: (311-324): Assertion violation happens here
