pragma experimental SMTChecker;

contract C {
    function f(int32 a, uint32 b) internal pure returns (int256) {
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
// ----
// Warning 6328: (242-269): CHC: Assertion violation happens here.
// Warning 6328: (344-373): CHC: Assertion violation happens here.
// Warning 6328: (447-474): CHC: Assertion violation happens here.
// Warning 6328: (547-572): CHC: Assertion violation happens here.
// Warning 6328: (645-670): CHC: Assertion violation happens here.
// Warning 6328: (743-768): CHC: Assertion violation happens here.
