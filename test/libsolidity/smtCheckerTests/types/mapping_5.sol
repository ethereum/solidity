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
// Warning 6328: (92-111): CHC: Assertion violation happens here.\nCounterexample:\n\na = 38\nx = 0\n\nTransaction trace:\nC.constructor()\nC.f(38, 0)
