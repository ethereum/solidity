pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) map;
	function f(uint x, uint y) public view {
		assert(x == y);
		assert(map[x] == map[y]);
	}
}
// ----
// Warning: (119-133): Assertion violation happens here
// Warning: (137-161): Assertion violation happens here
