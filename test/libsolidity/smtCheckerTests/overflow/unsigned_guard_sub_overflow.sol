contract C  {
	function f(uint x, uint y) public pure returns (uint) {
		require(x >= y);
		return x - y;
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
