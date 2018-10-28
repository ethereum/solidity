pragma experimental SMTChecker;

contract C
{
	mapping (bool => bool) map;
	function f(bool x) public view {
		assert(x != map[true]);
	}
}
// ----
// Warning: (111-133): Assertion violation happens here
