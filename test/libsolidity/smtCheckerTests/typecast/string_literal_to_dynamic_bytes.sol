contract C {
	function f() public pure {
		bytes memory b = bytes(hex"ffff");
		assert(b.length == 2); // should hold
		assert(b[0] == bytes1(uint8(255))); // should hold
		assert(b[1] == bytes1(uint8(100))); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (173-207): CHC: Assertion violation happens here.\nCounterexample:\n\nb = [0xff, 0xff]\n\nTransaction trace:\nC.constructor()\nC.f()
