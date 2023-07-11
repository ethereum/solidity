contract C
{
	function g() public pure {
		uint x;
		uint y;
		(x, y) = (2, 4);
		assert(x == 2);
		assert(y == 4);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
