contract C {
	function f() public pure {
		assert(addmod(2**256 - 1, 10, 9) == 7);
		uint y = 0;
		uint x = addmod(2**256 - 1, 10, y);
		assert(x == 1);
	}
	function g(uint x, uint y, uint k) public pure returns (uint) {
		return addmod(x, y, k);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 4281: (108-133): CHC: Division by zero happens here.\nCounterexample:\n\ny = 0\nx = 0\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (137-151): CHC: Assertion violation happens here.\nCounterexample:\n\ny = 0\nx = 0\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 4281: (230-245): CHC: Division by zero happens here.\nCounterexample:\n\nx = 0\ny = 0\nk = 0\n = 0\n\nTransaction trace:\nC.constructor()\nC.g(0, 0, 0)
