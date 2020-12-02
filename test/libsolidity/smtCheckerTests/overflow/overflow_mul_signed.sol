pragma experimental SMTChecker;

contract C
{
	function f(int8 x) public pure returns (int8) {
		x = 100;
		int8 y = x * 2;
		assert(y == -56);
		y = x * 100;
		assert(y == 16);
		return y;
	}
}
// ----
// Warning 4984: (117-122): CHC: Overflow (resulting value larger than 127) happens here.\nCounterexample:\n\nx = 100\n = 0\n\nTransaction trace:\nconstructor()\nf(0)
// Warning 4984: (150-157): CHC: Overflow (resulting value larger than 127) happens here.\nCounterexample:\n\nx = 100\n = 0\n\nTransaction trace:\nconstructor()\nf(0)
