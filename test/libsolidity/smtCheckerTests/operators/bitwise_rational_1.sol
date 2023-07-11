contract C {
	function f() public pure {
		uint x = type(uint256).max - 1;
		assert(x == 2**256 - 2);
		assert(~1 == -2);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
