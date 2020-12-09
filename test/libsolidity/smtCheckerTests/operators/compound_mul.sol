pragma experimental SMTChecker;

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
// ----
// Warning 6328: (150-164): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\n\n\nTransaction trace:\nconstructor()\nf(0)
