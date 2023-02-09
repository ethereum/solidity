contract C
{
	modifier m(uint x) {
		require(x == 2);
		_;
		return;
	}

	modifier n(uint x) {
		require(x == 3);
		_;
	}

	function f(uint x) m(x) n(x) public pure {
		assert(x == 3);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
