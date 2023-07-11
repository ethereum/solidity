contract C {
	function f(int x, int y) public pure returns (int) {
		require(y != 0);
		require(y != -1);
		return x / y;
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
