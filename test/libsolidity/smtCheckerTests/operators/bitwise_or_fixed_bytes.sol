pragma experimental SMTChecker;
contract C {
	function f() public pure returns (byte) {
		byte b = (byte(0x0F) | (byte(0xF0)));
		assert(b == byte(0xFF)); // should hold
		assert(b == byte(0x00)); // should fail
		return b;
	}
}
// ----
// Warning 6328: (172-195): CHC: Assertion violation happens here.
