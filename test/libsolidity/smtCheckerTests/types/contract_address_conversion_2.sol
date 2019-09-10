pragma experimental SMTChecker;

contract C
{
	function f(C c, C d) public pure {
		assert(address(c) == address(c));
		address a = address(c);
		require(c == d);
		assert(a == address(d));
	}
}
