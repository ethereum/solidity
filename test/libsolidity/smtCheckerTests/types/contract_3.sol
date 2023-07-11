contract C
{
	function f(C c, C d, C e) public pure {
		require(c == d);
		require(d == e);
		assert(c == e);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
