contract C {
    function f(uint8 a, uint8 b) internal pure returns (uint256) {
        return a >> b;
    }
	function t() public pure {
		assert(f(0x66, 0) == 0x66);
		// Fails because the above is true.
		assert(f(0x66, 0) == 0x6);

		assert(f(0x66, 8) == 0);
		// Fails because the above is true.
		assert(f(0x66, 8) == 1);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (207-232): CHC: Assertion violation happens here.
// Warning 6328: (302-325): CHC: Assertion violation happens here.
