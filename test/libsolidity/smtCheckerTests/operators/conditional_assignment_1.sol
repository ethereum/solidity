pragma experimental SMTChecker;

contract C {
	function f(bool b) public pure {
		uint a = b ? 2 : 3;
		assert(a > 2);
	}
}
// ----
// Warning 6328: (104-117): CHC: Assertion violation happens here.\nCounterexample:\n\nb = true\n\n\nTransaction trace:\nconstructor()\nf(true)
