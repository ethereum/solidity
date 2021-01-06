pragma experimental SMTChecker;

contract C
{
	uint x;

	modifier m {
		if (x == 0)
			_;
	}

	function f() m public view {
		assert(x == 0);
		assert(x > 1);
	}
}
// ----
// Warning 6328: (144-157): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.f()
