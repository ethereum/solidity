contract C
{
	modifier m(uint a, uint b) {
		require(g(a, b));
		_;
	}

	modifier notZero(uint x) {
		require(x > 0);
		_;
	}

	function g(uint a, uint b) notZero(a) internal pure returns (bool) {
		return a > b;
	}

	function f(uint x) m(x, 0) public pure {
		assert(x > 0);
		assert(x > 1);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (278-291): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
