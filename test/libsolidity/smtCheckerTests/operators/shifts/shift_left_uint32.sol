pragma experimental SMTChecker;

contract C {
    function f(uint32 a, uint32 b) internal pure returns (uint256) {
        return a << b;
    }
	function t() public pure {
		assert(f(0x4266, 0) == 0x4266);
		// Fails because the above is true.
		assert(f(0x4266, 0) == 0x4267);

		assert(f(0x4266, 0x10) == 0x42660000);
		// Fails because the above is true.
		assert(f(0x4266, 0x10) == 0x426600000);

		assert(f(0x4266, 0x11) == 0x84cc0000);
		// Fails because the above is true.
		assert(f(0x4266, 0x11) == 0x84cc000);

		assert(f(0x4266, 0x20) == 0);
		// Fails because the above is true.
		assert(f(0x4266, 0x20) == 1);
	}
}
// ----
// Warning 6328: (246-276): CHC: Assertion violation happens here.
// Warning 6328: (360-398): CHC: Assertion violation happens here.
// Warning 6328: (482-518): CHC: Assertion violation happens here.
// Warning 6328: (593-621): CHC: Assertion violation happens here.
