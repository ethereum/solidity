contract C
{
	function f() public view {
		assert(gasleft() > 0);
		uint g = gasleft();
		assert(g < gasleft());
		assert(g >= gasleft());
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (43-64): CHC: Assertion violation happens here.\nCounterexample:\n\ng = 0\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (90-111): CHC: Assertion violation happens here.\nCounterexample:\n\ng = 0\n\nTransaction trace:\nC.constructor()\nC.f()
