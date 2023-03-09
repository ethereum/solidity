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
// Warning 6328: (76-90): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
