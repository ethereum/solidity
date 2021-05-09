contract C {
	function f(uint i) public pure {
		bytes4 x = 0x01020304;
		require(i > 3);
		assert(x[i] == 0x00);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6368: (99-103): CHC: Out of bounds access happens here.\nCounterexample:\n\ni = 4\nx = 16909060\n\nTransaction trace:\nC.constructor()\nC.f(4)
// Warning 6328: (92-112): CHC: Assertion violation happens here.\nCounterexample:\n\ni = 4\nx = 16909060\n\nTransaction trace:\nC.constructor()\nC.f(4)
