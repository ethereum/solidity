pragma experimental SMTChecker;

contract C
{
	mapping (bool => uint) map;
	function f(uint x) public view {
		assert(x != map[true]);
	}
}
// ----
// Warning: (111-133): Assertion violation happens here
