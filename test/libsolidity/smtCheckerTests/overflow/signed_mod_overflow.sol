pragma experimental SMTChecker;

contract C  {
	function f(int x, int y) public pure returns (int) {
		return x % y;
	}
}
// ----
// Warning 4281: (110-115): CHC: Division by zero happens here.\nCounterexample:\n\nx = 0\ny = 0\n = 0\n\nTransaction trace:\nconstructor()\nf(0, 0)
