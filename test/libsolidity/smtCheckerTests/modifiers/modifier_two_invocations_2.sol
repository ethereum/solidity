pragma experimental SMTChecker;

contract C
{
	uint x;

	modifier m {
		require(x == 0);
		_;
		x = x + 1;
		assert(x <= 2);
	}

	function f() m m public {
		x = x + 1;
	}
}
// ----
// Warning 6328: (109-123): CHC: Assertion violation happens here.\nCounterexample:\nx = 3\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\nf()
