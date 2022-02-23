contract C  {
	function f(uint x, uint y) public pure returns (uint) {
		return x - y;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 3944: (80-85): CHC: Underflow (resulting value less than 0) happens here.\nCounterexample:\n\nx = 0\ny = 1\n = 0\n\nTransaction trace:\nC.constructor()\nC.f(0, 1)
