contract C {
	function f(uint x, uint y) public pure returns (uint) {
		return x / y;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 4281: (79-84): CHC: Division by zero happens here.\nCounterexample:\n\nx = 0\ny = 0\n = 0\n\nTransaction trace:\nC.constructor()\nC.f(0, 0)
