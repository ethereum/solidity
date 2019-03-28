pragma experimental SMTChecker;

contract C
{
	function f(uint x) public pure {
		require(x < 10);
		uint y = 10;
		y *= y + x;
		assert(y <= 190);
		assert(y < 50);
	}
}
// ----
// Warning: (150-164): Assertion violation happens here
