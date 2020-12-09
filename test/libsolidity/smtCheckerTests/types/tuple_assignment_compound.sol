pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		uint a = 1;
		uint b = 3;
		a += ((((b))));
		assert(a == 3);
	}
}
// ----
// Warning 6328: (122-136): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
