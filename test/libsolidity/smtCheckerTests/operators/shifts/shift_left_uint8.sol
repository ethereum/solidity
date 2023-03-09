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
// ====
// SMTEngine: all
// ----
// Warning 6328: (209-238): CHC: Assertion violation happens here.
// Warning 6328: (310-335): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
