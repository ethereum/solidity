contract C
{
	function f(C c, C d) public pure {
		assert(address(c) == address(c));
		address a = address(c);
		require(c == d);
		assert(a == address(d));
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
