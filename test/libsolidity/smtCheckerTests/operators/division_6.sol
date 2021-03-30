pragma experimental SMTChecker;
contract C {
	function mul(uint8 a, uint8 b) public pure returns (uint8) {
		if (a == 0) {
			return 0;
		}
		uint8 c = a * b;
		require(c / a == b);
		return c;
	}
}
// ----
// Warning 4984: (152-157): CHC: Overflow (resulting value larger than 255) happens here.\nCounterexample:\n\na = 128\nb = 2\n = 0\nc = 0\n\nTransaction trace:\nC.constructor()\nC.mul(128, 2)
// Warning 6838: (169-179): BMC: Condition is always true.
