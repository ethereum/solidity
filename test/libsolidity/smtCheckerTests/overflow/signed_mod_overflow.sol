contract C  {
	function f(int x, int y) public pure returns (int) {
		return x % y;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 4281: (77-82): CHC: Division by zero happens here.\nCounterexample:\n\nx = 0\ny = 0\n = 0\n\nTransaction trace:\nC.constructor()\nC.f(0, 0)
