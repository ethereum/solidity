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
// Warning: (117-122): Underflow (resulting value less than 0) happens here
// Warning: (150-157): Underflow (resulting value less than 0) happens here
