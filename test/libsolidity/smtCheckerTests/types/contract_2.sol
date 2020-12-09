pragma experimental SMTChecker;

contract D
{
	uint x;
}

contract C
{
	function f(D c, D d) public pure {
		assert(c == d);
	}
}
// ----
// Warning 6328: (109-123): CHC: Assertion violation happens here.\nCounterexample:\n\nc = 0\nd = 1\n\n\nTransaction trace:\nconstructor()\nf(0, 1)
