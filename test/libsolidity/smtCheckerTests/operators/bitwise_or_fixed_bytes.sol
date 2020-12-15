pragma experimental SMTChecker;
contract C {
	function f() public pure returns (bytes1) {
		bytes1 b = (bytes1(0x0F) | (bytes1(0xF0)));
		assert(b == bytes1(0xFF)); // should hold
		assert(b == bytes1(0x00)); // should fail
		return b;
	}
}
// ----
// Warning 6328: (182-207): CHC: Assertion violation happens here.\nCounterexample:\n\n\n = 0\n\nTransaction trace:\nconstructor()\nf()
