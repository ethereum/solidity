pragma experimental SMTChecker;

contract C
{
	mapping (uint => bool) map;
	function f(bool x) public view {
		assert(x != map[2]);
	}
}
// ----
// Warning: (111-130): Assertion violation happens here
