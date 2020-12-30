pragma experimental SMTChecker;
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
// ----
// Warning 6838: (161-171): BMC: Condition is always true.
