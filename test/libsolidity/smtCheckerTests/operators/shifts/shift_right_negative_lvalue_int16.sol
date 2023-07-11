contract C {
    function f(int16 a, uint16 b) internal pure returns (int256) {
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
// Warning 6328: (209-236): CHC: Assertion violation happens here.
// Warning 6328: (311-340): CHC: Assertion violation happens here.
// Warning 6328: (414-441): CHC: Assertion violation happens here.
// Warning 6328: (514-539): CHC: Assertion violation happens here.
// Warning 6328: (612-637): CHC: Assertion violation happens here.
// Warning 6328: (710-735): CHC: Assertion violation happens here.
// Info 1391: CHC: 6 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
