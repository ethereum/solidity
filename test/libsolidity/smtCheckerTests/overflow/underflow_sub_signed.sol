pragma experimental SMTChecker;

contract C
{
	function f(int8 x) public pure returns (int8) {
		x = -2;
		int8 y = x - 127;
		assert(y == 127);
		x = -128;
		y = x - 127;
		assert(y == 1);
		x = 127;
		y = x - (-127);
		assert(y == -2);
		return y;
	}
}
// ----
// Warning: (116-123): Underflow (resulting value less than -128) happens here
// Warning: (163-170): Underflow (resulting value less than -128) happens here
// Warning: (207-217): Overflow (resulting value larger than 127) happens here
