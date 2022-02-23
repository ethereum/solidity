contract C
{
	mapping (uint => bool) map;
	function f(bool x) public view {
		assert(x != map[2]);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (78-97): CHC: Assertion violation happens here.\nCounterexample:\n\nx = false\n\nTransaction trace:\nC.constructor()\nC.f(false)
