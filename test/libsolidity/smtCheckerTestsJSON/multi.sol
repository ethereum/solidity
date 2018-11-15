pragma experimental SMTChecker;

contract C
{
	function f(uint x) public pure {
		assert(x > 0);
		assert(x > 100);
		assert(x >= 0);
	}
}
// ----
// Warning: (82-95): Assertion violation happens here
// Warning: (99-114): Assertion violation happens here
