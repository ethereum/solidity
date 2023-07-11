contract C
{
	uint x;

	modifier m {
		// Condition is always true for the second invocation.
		require(x > 0);
		require(x < 10000);
		_;
		assert(x > 1);
	}

	function f() m m public {
		x = x + 1;
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
