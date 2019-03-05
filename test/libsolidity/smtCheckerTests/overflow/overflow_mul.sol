pragma experimental SMTChecker;

contract C
{
	function f(uint8 x) public pure returns (uint8) {
		x = 100;
		uint8 y = x * 3;
		assert(y == 44);
		x = 128;
		y = x * 4;
		assert(y == 0);
		return y;
	}
}
// ----
// Warning: (120-125): Overflow (resulting value larger than 255) happens here
// Warning: (163-168): Overflow (resulting value larger than 255) happens here
