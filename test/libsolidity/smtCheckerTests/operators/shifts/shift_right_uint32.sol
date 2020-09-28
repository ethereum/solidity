pragma experimental SMTChecker;

contract C {
    function f(uint32 a, uint32 b) internal pure returns (uint256) {
        return a >> b;
    }
	function t() public pure {
		assert(f(0x4266, 0) == 0x4266);
		// Fails because the above is true.
		assert(f(0x4266, 0) == 0x426);

		assert(f(0x4266, 8) == 0x42);
		// Fails because the above is true.
		assert(f(0x4266, 8) == 0x420);

		assert(f(0x4266, 0x10) == 0);
		// Fails because the above is true.
		assert(f(0x4266, 0x10) == 255);

		assert(f(0x4266, 0x11) == 0);
		// Fails because the above is true.
		assert(f(0x4266, 0x11) == 255);
	}
}
// ----
// Warning 6328: (246-275): CHC: Assertion violation happens here.
// Warning 6328: (350-379): CHC: Assertion violation happens here.
// Warning 6328: (454-484): CHC: Assertion violation happens here.
// Warning 6328: (559-589): CHC: Assertion violation happens here.
