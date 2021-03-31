contract C {
    function f(int256 a, uint256 b) internal pure returns (int256) {
        return a >> b;
    }
	function t() public pure {
		assert(f(-4266, 0) == -4266);
		// Fails because the above is true.
		assert(f(-4266, 0) == -426);

		assert(f(-4266, 1) == -2133);
		// Fails because the above is true.
		assert(f(-4266, 1) == -21330);

		assert(f(-4266, 4) == -267);
		// Fails because the above is true.
		assert(f(-4266, 4) == -255);

		assert(f(-4266, 8) == -17);
		// Fails because the above is true.
		assert(f(-4266, 8) == -1);

		assert(f(-4266, 16) == -1);
		// Fails because the above is true.
		assert(f(-4266, 16) == 0);

		assert(f(-4266, 17) == -1);
		// Fails because the above is true.
		assert(f(-4266, 17) == 0);
	}

}
// ====
// SMTEngine: all
// ----
// Warning 6328: (211-238): CHC: Assertion violation happens here.
// Warning 6328: (313-342): CHC: Assertion violation happens here.
// Warning 6328: (416-443): CHC: Assertion violation happens here.
// Warning 6328: (516-541): CHC: Assertion violation happens here.
// Warning 6328: (614-639): CHC: Assertion violation happens here.
// Warning 6328: (712-737): CHC: Assertion violation happens here.
