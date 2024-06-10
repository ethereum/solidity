contract C {
	function f(int x, int y) public pure returns (int) {
		require(x == 0);
		require(y == 0);
		return x + y;
	}
}
// ====
// SMTEngine: chc
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
