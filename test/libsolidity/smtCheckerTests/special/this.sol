pragma experimental SMTChecker;

contract C
{
	function f(address a) public view {
		assert(a == address(this));
	}
}
// ----
// Warning 6328: (85-111): CHC: Assertion violation happens here.\nCounterexample:\n\na = 0\n\n\nTransaction trace:\nconstructor()\nf(0)
