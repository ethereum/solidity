contract C {
	function f() public pure returns (bytes1) {
		bytes1 b = (bytes1(0x0F) | (bytes1(0xF0)));
		assert(b == bytes1(0xFF)); // should hold
		assert(b == bytes1(0x00)); // should fail
		return b;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (150-175): CHC: Assertion violation happens here.\nCounterexample:\n\n = 0\nb = 255\n\nTransaction trace:\nC.constructor()\nC.f()
