contract C {
    function f(uint256 a, uint256 b) internal pure returns (uint256) {
        return a >> b;
    }
	function t() public pure {
		assert(f(0x4266, 0) == 0x4266);
		// Fails because the above is true.
		assert(f(0x4266, 0) == 0x426);

		assert(f(0x4266, 0x8) == 0x42);
		// Fails because the above is true.
		assert(f(0x4266, 0x8) == 0x420);

		assert(f(0x4266, 0x11) == 0);
		// Fails because the above is true.
		assert(f(0x4266, 0x11) == 1);

		assert(f(57896044618658097711785492504343953926634992332820282019728792003956564819968, 5) == 1809251394333065553493296640760748560207343510400633813116524750123642650624);
		// Fails because the above is true.
		assert(f(57896044618658097711785492504343953926634992332820282019728792003956564819968, 5) == 0);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (215-244): CHC: Assertion violation happens here.
// Warning 6328: (321-352): CHC: Assertion violation happens here.
// Warning 6328: (427-455): CHC: Assertion violation happens here.
// Warning 6328: (673-769): CHC: Assertion violation happens here.
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
