contract C
{
	function g() public pure {
		(uint x, ) = (2, 4);
		assert(x == 2);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
