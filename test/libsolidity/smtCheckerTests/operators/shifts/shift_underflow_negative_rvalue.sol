pragma experimental SMTChecker;

contract C {
    function f(int256 a, uint256 b) internal pure returns (int256) {
        return a << b;
    }

    function g(int256 a, uint256 b) internal pure returns (int256) {
        return a >> b;
    }

	function t() public pure {
		assert(f(1, 2**256 - 1) == 0);
		// Fails because the above is true.
		assert(f(1, 2**256 - 1) == 1);

		assert(g(1, 2**256 - 1) == 0);
		// Fails because the above is true.
		assert(g(1, 2**256 - 1) == 1);
	}
}
// ----
// Warning 6328: (345-374): CHC: Assertion violation happens here.
// Warning 6328: (450-479): CHC: Assertion violation happens here.
