pragma experimental SMTChecker;

contract C {
    function f(uint256 a, uint256 b) internal pure returns (uint256) {
        return a << b;
    }
	function t() public pure {
		assert(f(0x4266, 0x0) == 0x4266);
		// Fails because the above is true.
		assert(f(0x4266, 0x0) == 0x4268);

		assert(f(0x4266, 0x8) == 0x426600);
		// Fails because the above is true.
		assert(f(0x4266, 0x8) == 0x120939);

		assert(f(0x4266, 0xf0) == 0x4266000000000000000000000000000000000000000000000000000000000000);
		// Fails because the above is true.
		assert(f(0x4266, 0xf0) == 0x4266000000000000000000000000000000000000000000000000000000000001);

		assert(f(0x4266, 0x4266) == 0);
		// Fails because the above is true.
		assert(f(0x4266, 0x4266) == 1);
	}
}
// ----
// Warning 6328: (250-282): CHC: Assertion violation happens here.
// Warning 6328: (363-397): CHC: Assertion violation happens here.
// Warning 6328: (537-630): CHC: Assertion violation happens here.
// Warning 6328: (707-737): CHC: Assertion violation happens here.
