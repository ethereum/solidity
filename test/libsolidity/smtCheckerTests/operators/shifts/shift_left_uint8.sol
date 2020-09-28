pragma experimental SMTChecker;

contract C {
    function f(uint8 a, uint8 b) internal pure returns (uint256) {
        return a << b;
    }
	function t() public pure {
		assert(f(0x66, 0x0) == 0x66);
		// Fails because the above is true.
		assert(f(0x66, 0x0) == 0x660);

		assert(f(0x66, 0x8) == 0);
		// Fails because the above is true.
		assert(f(0x66, 0x8) == 1);
	}
}
// ----
// Warning 6328: (242-271): CHC: Assertion violation happens here.
// Warning 6328: (343-368): CHC: Assertion violation happens here.
