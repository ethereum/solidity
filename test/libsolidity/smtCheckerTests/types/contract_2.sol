pragma experimental SMTChecker;

contract D
{
	uint x;
}

contract C
{
	function f(D c, D d) public pure {
		assert(c == d);
	}
}
// ----
// Warning: (109-123): Assertion violation happens here
