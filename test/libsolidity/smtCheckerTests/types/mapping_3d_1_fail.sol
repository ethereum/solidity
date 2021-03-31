contract C
{
	mapping (uint => mapping (uint => mapping (uint => uint))) map;
	function f(uint x) public {
		x = 41;
		map[13][14][15] = 42;
		assert(x == map[13][14][15]);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (143-171): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 41\n\nTransaction trace:\nC.constructor()\nC.f(0)
