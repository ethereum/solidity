pragma experimental SMTChecker;

contract C
{
	function f(uint8 x) public pure returns (uint) {
		x = 0;
		uint8 y = x - 1;
		assert(y == 255);
		y = x - 255;
		assert(y == 1);
		return y;
	}
}
// ----
// Warning 3944: (117-122): CHC: Underflow (resulting value less than 0) happens here.\nCounterexample:\n\nx = 0\n = 0\n\nTransaction trace:\nconstructor()\nf(0)
// Warning 3944: (150-157): CHC: Underflow (resulting value less than 0) happens here.\nCounterexample:\n\nx = 0\n = 0\n\nTransaction trace:\nconstructor()\nf(0)
