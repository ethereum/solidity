pragma experimental SMTChecker;

contract C {
	function f(int16 x, uint16 y, int16 z) internal pure returns (bool) {
		return x >> y == z;
	}

	function t() public pure {
		assert(f(-4266, 0, -4266));
		// Fails because the above is true.
		assert(f(-4266, 0, -426));

		assert(f(-4266, 1, -2133));
		// Fails because the above is true.
		assert(f(-4266, 1, -2134));

		assert(f(-4266, 4, -267));
		// Fails because the above is true.
		assert(f(-4266, 4, -2670));

		assert(f(-4266, 8, -17));
		// Fails because the above is true.
		assert(f(-4266, 8, -1));

		assert(f(-4266, 16, -1));
		// Fails because the above is true.
		assert(f(-4266, 16, -0));

		assert(f(-4266, 17, -1));
		// Fails because the above is true.
		assert(f(-4266, 17, -0));
	}
}
// ----
// Warning 6328: (241-266): CHC: Assertion violation happens here.
// Warning 6328: (339-365): CHC: Assertion violation happens here.
// Warning 6328: (437-463): CHC: Assertion violation happens here.
// Warning 6328: (534-557): CHC: Assertion violation happens here.
// Warning 6328: (628-652): CHC: Assertion violation happens here.
// Warning 6328: (723-747): CHC: Assertion violation happens here.
