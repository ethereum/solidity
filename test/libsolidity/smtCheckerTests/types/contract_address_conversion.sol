pragma experimental SMTChecker;

contract C
{
	function f(C c, address a) public pure {
		assert(address(c) == a);
	}
}
// ----
// Warning 6328: (90-113): CHC: Assertion violation happens here.\nCounterexample:\n\nc = 0\na = 1\n\n\nTransaction trace:\nC.constructor()\nC.f(0, 1)
