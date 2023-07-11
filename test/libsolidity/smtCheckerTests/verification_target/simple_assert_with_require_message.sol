contract C {
	function f(uint a) public pure {
		require(a < 10, "Input number is too large.");
		assert(a < 20);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
