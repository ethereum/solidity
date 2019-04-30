pragma experimental SMTChecker;

contract C
{
	function f(C c, C d, C e) public pure {
		require(c == d);
		require(d == e);
		assert(c == e);
	}
}
