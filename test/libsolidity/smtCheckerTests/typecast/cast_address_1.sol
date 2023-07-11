contract C
{
	function f(address a) public pure {
		require(a != address(0));
		assert(a != address(0));
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
