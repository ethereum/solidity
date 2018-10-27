pragma experimental SMTChecker;

contract C
{
	mapping (bool => bool) map;
	function f(bool x) public view {
		assert(x != map[true]);
	}
}
