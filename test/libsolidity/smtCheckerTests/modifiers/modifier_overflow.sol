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
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
