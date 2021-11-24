contract C
{
	mapping (uint => uint) map;
	function f(uint x, uint p) public {
		require(x < 100);
		map[p] = 100;
		map[p] += map[p] + x;
		assert(map[p] < 300);
		assert(map[p] < 110);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6328: (165-185): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\np = 0\n\nTransaction trace:\nC.constructor()\nC.f(0, 0)
