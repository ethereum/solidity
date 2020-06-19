pragma experimental SMTChecker;

contract C
{
	function f(C c, C d) public pure {
		assert(c == d);
	}
}
// ----
// Warning 4661: (84-98): Assertion violation happens here
