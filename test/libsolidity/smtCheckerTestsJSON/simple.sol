pragma experimental SMTChecker;

contract C
{
	function f(uint x) public pure {
		assert(x > 0);
	}
}
// ----
// Warning: (82-95): Assertion violation happens here
