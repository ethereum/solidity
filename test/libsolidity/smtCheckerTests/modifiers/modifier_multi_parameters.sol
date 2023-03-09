contract C
{
	modifier m(uint a, uint b) {
		require(a > b);
		_;
	}

	function f(uint x) m(x, 0) public pure {
		assert(x > 0);
		assert(x > 1);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (131-144): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
