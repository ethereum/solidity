contract C
{
	modifier m(uint a, uint b) {
		require(g(a, b));
		_;
	}

	function g(uint a, uint b) m(a, b) internal pure returns (bool) {
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
// Warning 5740: (137-157): Unreachable code.
// Warning 5740: (199-237): Unreachable code.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
