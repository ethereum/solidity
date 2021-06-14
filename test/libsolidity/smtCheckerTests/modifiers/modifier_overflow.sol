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
// Warning 0: (0-123): Contract invariants for :C:\n(x <= 0)\n
