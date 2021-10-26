contract C
{
	uint x;

	modifier m {
		require(x > 0);
		_;
	}

	function f() m public {
		assert(x > 0);
		x = x + 1;
	}
}
// ====
// SMTEngine: all
// ----
// Info 1180: Contract invariant(s) for :C:\n(x <= 0)\n
