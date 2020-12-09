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
// Warning 6328: (119-133): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\ny = 1\n\n\nTransaction trace:\nconstructor()\nf(0, 1)
