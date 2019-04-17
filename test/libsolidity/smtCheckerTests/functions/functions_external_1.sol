pragma experimental SMTChecker;

contract D
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
// Warning: (240-254): Assertion violation happens here
