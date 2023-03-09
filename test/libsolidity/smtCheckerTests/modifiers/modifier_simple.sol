contract C
{
	uint x;

	modifier m {
		require(x > 0);
		_;
	}

	function f() m public view {
		assert(x > 0);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
