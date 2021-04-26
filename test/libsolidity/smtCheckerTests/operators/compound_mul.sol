contract C
{
	function f(uint x) public pure {
		require(x < 10);
		uint y = 10;
		y *= y + x;
		assert(y <= 190);
		assert(y < 50);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (117-131): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\ny = 100\n\nTransaction trace:\nC.constructor()\nC.f(0)
