pragma experimental SMTChecker;

contract C
{
	mapping (uint => bool) map;
	function f(bool x) public view {
		assert(x != map[2]);
	}
}
// ----
// Warning 6328: (111-130): CHC: Assertion violation happens here.\nCounterexample:\n\nx = false\n\n\nTransaction trace:\nconstructor()\nf(false)
