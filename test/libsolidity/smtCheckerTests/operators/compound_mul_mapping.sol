contract C
{
	mapping (uint => uint) map;
	function f(uint x, uint p) public {
		require(x < 10);
		map[p] = 10;
		map[p] *= map[p] + x;
		assert(map[p] <= 190);
		assert(map[p] < 50);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (164-183): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\np = 0\n\nTransaction trace:\nC.constructor()\nC.f(0, 0)
