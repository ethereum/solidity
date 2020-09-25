pragma experimental SMTChecker;

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
// ----
// Warning 6328: (244-271): CHC: Assertion violation happens here.
// Warning 6328: (346-375): CHC: Assertion violation happens here.
// Warning 6328: (449-476): CHC: Assertion violation happens here.
// Warning 6328: (549-574): CHC: Assertion violation happens here.
// Warning 6328: (647-672): CHC: Assertion violation happens here.
// Warning 6328: (745-770): CHC: Assertion violation happens here.
