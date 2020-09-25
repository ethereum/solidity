pragma experimental SMTChecker;

contract C
{
	function f(int8 x) public pure returns (int8) {
		x = 127;
		int8 y = x + 1;
		assert(y == -128);
		y = x + 127;
		assert(y == -2);
		x = -127;
		y = x + -127;
		assert(y == 2);
	}
}
// ----
// Warning 4984: (117-122): CHC: Overflow (resulting value larger than 127) happens here.
// Warning 4984: (151-158): CHC: Overflow (resulting value larger than 127) happens here.
// Warning 3944: (197-205): CHC: Underflow (resulting value less than -128) happens here.
