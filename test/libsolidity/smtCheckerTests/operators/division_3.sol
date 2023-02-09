contract C {
	function f(int x, int y) public pure returns (int) {
		require(y != 0);
		return x / y;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 4984: (95-100): CHC: Overflow (resulting value larger than 2**255 - 1) happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
