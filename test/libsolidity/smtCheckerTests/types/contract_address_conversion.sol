pragma experimental SMTChecker;

contract C
{
	function f(C c, address a) public pure {
		assert(address(c) == a);
	}
}
// ----
// Warning 6328: (90-113): CHC: Assertion violation happens here.\nCounterexample:\n\nc = 1\na = 0\n\n\nTransaction trace:\nconstructor()\nf(1, 0)
