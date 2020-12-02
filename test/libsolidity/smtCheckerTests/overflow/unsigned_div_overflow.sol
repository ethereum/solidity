pragma experimental SMTChecker;

contract C  {
	function f(uint x, uint y) public pure returns (uint) {
		return x / y;
	}
}
// ----
// Warning 4281: (113-118): CHC: Division by zero happens here.\nCounterexample:\n\nx = 0\ny = 0\n = 0\n\nTransaction trace:\nconstructor()\nf(0, 0)
