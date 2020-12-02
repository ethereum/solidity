pragma experimental SMTChecker;

contract C  {
	function f(uint x, uint y) public pure returns (uint) {
		return x - y;
	}
}
// ----
// Warning 3944: (113-118): CHC: Underflow (resulting value less than 0) happens here.\nCounterexample:\n\nx = 0\ny = 1\n = 0\n\nTransaction trace:\nconstructor()\nf(0, 1)
