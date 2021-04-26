contract C
{
	mapping (uint => uint) map;
	function f(uint x, uint y) public view {
		assert(x == y);
		assert(map[x] == map[y]);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (86-100): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\ny = 1\n\nTransaction trace:\nC.constructor()\nC.f(0, 1)
