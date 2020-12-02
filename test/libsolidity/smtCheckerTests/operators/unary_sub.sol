pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		uint x = 5;
		uint a = --x;
		assert(x == 4);
		assert(a == 4);
		uint b = x--;
		assert(x == 3);
		// Should fail.
		assert(b > 4);
	}
}
// ----
// Warning 6328: (194-207): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
