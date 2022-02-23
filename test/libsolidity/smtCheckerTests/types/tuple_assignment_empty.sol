contract C
{
	function g() public pure {
		uint x;
		uint y;
		(x, ) = (2, 4);
		assert(x == 2);
		assert(y == 4);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (99-113): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 2\ny = 0\n\nTransaction trace:\nC.constructor()\nC.g()
