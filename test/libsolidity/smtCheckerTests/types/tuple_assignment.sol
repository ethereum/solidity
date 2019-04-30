pragma experimental SMTChecker;

contract C
{
	function g() public pure {
		uint x;
		uint y;
		(x, y) = (2, 4);
		assert(x == 1);
		assert(0 == 1);
	}
}
// ----
// Warning: (115-129): Assertion violation happens here
