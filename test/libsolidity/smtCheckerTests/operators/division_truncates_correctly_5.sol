contract C {
	function f(int x, int y) public pure {
		x = -7;
		y = -2;
		assert(x / y == 3);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
