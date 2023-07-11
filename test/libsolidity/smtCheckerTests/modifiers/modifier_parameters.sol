contract C
{
	uint s;
	modifier m(uint a) {
		// Condition is always true for m(2).
		require(a > 0);
		_;
	}

	function f(uint x) m(x) m(2) m(s) public view {
		assert(x > 0);
		assert(s > 0);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
