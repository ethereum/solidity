pragma experimental SMTChecker;

contract C {
    function leftU(uint8 x, uint8 y) internal pure returns (uint8) {
        return x << y;
    }

    function leftS(int8 x, uint8 y) internal pure returns (int8) {
        return x << y;
    }

	function t() public pure {
		assert(leftU(255, 8) == 0);
		// Fails because the above is true.
		assert(leftU(255, 8) == 1);

		assert(leftU(255, 1) == 254);
		// Fails because the above is true.
		assert(leftU(255, 1) == 255);

		assert(leftU(255, 0) == 255);
		// Fails because the above is true.
		assert(leftU(255, 0) == 0);

		assert(leftS(1, 7) == -128);
		// Fails because the above is true.
		assert(leftS(1, 7) == 127);

		assert(leftS(1, 6) == 64);
		// Fails because the above is true.
		assert(leftS(1, 6) == -64);
	}
}
// ----
// Warning 6328: (340-366): CHC: Assertion violation happens here.
// Warning 6328: (441-469): CHC: Assertion violation happens here.
// Warning 6328: (544-570): CHC: Assertion violation happens here.
// Warning 6328: (644-670): CHC: Assertion violation happens here.
// Warning 6328: (742-768): CHC: Assertion violation happens here.
