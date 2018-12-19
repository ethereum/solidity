pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) map;
	function f(uint x, uint y) public view {
		require(x == y);
		assert(map[x] == map[y]);
	}
}
