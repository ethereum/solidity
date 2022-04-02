contract C
{
	mapping (address => uint) map;
	function f(address a, uint x) public view {
		assert(x != map[a]);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (92-111='assert(x != map[a])'): CHC: Assertion violation happens here.\nCounterexample:\n\na = 0x0\nx = 0\n\nTransaction trace:\nC.constructor()\nC.f(0x0, 0)
