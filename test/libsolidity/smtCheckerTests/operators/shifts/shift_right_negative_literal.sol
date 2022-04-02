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
// ====
// SMTEngine: all
// ----
// Warning 6328: (208-233='assert(f(-4266, 0, -426))'): CHC: Assertion violation happens here.
// Warning 6328: (306-332='assert(f(-4266, 1, -2134))'): CHC: Assertion violation happens here.
// Warning 6328: (404-430='assert(f(-4266, 4, -2670))'): CHC: Assertion violation happens here.
// Warning 6328: (501-524='assert(f(-4266, 8, -1))'): CHC: Assertion violation happens here.
// Warning 6328: (595-619='assert(f(-4266, 16, -0))'): CHC: Assertion violation happens here.
// Warning 6328: (690-714='assert(f(-4266, 17, -0))'): CHC: Assertion violation happens here.
