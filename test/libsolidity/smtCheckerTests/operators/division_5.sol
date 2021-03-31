contract C {
	function mul(uint8 a, uint8 b) internal pure returns (uint8) {
		uint8 c;
		if (a != 0) {
			c = a * b;
			require(c / a == b);
		}
		return c;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6838: (129-139): BMC: Condition is always true.
