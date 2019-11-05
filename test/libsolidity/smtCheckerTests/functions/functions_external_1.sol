pragma experimental SMTChecker;

abstract contract D
{
	function g(uint x) public;
}

contract C
{
	uint x;
	function f(uint y, D d) public {
		require(x == y);
		assert(x == y);
		d.g(y);
		// Storage knowledge is cleared after an external call.
		assert(x == y);
	}
}
// ----
// Warning: (249-263): Assertion violation happens here
