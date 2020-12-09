pragma experimental SMTChecker;

contract C
{
	function f(uint x) public pure {
		require(x < 100);
		uint y = 100;
		y += y + x;
		assert(y < 300);
		assert(y < 110);
	}
}
// ----
// Warning 6328: (151-166): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\n\n\nTransaction trace:\nconstructor()\nf(0)
