contract C
{
	function f(uint x) public pure {
		require(x < 10000);
		uint y = x * 2;
		assert((y % 2) == 0);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
