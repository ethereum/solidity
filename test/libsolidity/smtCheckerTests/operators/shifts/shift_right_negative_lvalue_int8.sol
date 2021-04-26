contract C {
    function f(int8 a, uint8 b) internal pure returns (int256) {
        return a >> b;
    }
	function t() public pure {
		assert(f(-66, 0) == -66);
		// Fails because the above is true.
		assert(f(-66, 0) == -6);

		assert(f(-66, 1) == -33);
		// Fails because the above is true.
		assert(f(-66, 1) == -3);

		assert(f(-66, 4) == -5);
		// Fails because the above is true.
		assert(f(-66, 4) == -2);

		assert(f(-66, 8) == -1);
		// Fails because the above is true.
		assert(f(-66, 8) == -2);

		assert(f(-66, 16) == -1);
		// Fails because the above is true.
		assert(f(-66, 16) == 0);

		assert(f(-66, 17) == -1);
		// Fails because the above is true.
		assert(f(-66, 17) == 0);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (203-226): CHC: Assertion violation happens here.
// Warning 6328: (297-320): CHC: Assertion violation happens here.
// Warning 6328: (390-413): CHC: Assertion violation happens here.
// Warning 6328: (483-506): CHC: Assertion violation happens here.
// Warning 6328: (577-600): CHC: Assertion violation happens here.
// Warning 6328: (671-694): CHC: Assertion violation happens here.
