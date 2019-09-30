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
// Warning: (47-192): Error trying to invoke SMT solver.
// Warning: (117-122): Overflow (resulting value larger than 127) happens here
// Warning: (150-157): Overflow (resulting value larger than 127) happens here
