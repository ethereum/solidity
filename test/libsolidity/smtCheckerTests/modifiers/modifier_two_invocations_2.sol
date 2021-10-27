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
// ====
// SMTEngine: all
// ----
// Warning 6328: (76-90): CHC: Assertion violation happens here.\nCounterexample:\nx = 3\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.f()
// Info 1180: Contract invariant(s) for :C:\n(x = 3)\n
