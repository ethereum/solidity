pragma experimental SMTChecker;

contract C
{
	mapping (bool => bool) map;
	function f(bool x) public view {
		require(x);
		assert(x != map[x]);
	}
}
// ----
