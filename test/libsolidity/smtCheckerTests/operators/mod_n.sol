contract C
{
	function f(uint x, uint y) public pure {
		require(y > 0);
		uint z = x % y;
		assert(z < y);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
