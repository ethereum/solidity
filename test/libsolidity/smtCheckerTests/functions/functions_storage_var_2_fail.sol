pragma experimental SMTChecker;
contract C
{
	uint a;
	function f(uint x) public {
		uint y;
		a = (y = x);
	}
	function g() public {
		f(1);
		f(0);
		assert(a > 0);
	}
}

// ----
// Warning 6328: (152-165): CHC: Assertion violation happens here.\nCounterexample:\na = 0\n\n\n\nTransaction trace:\nconstructor()\nState: a = 0\ng()
