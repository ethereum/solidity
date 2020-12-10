pragma experimental SMTChecker;

contract C
{
	function f(C c, C d) public pure {
		assert(c == d);
	}
}
// ----
// Warning 6328: (84-98): CHC: Assertion violation happens here.\nCounterexample:\n\nc = 0\nd = 1\n\n\nTransaction trace:\nconstructor()\nf(0, 1)
