contract C {
	function f(uint x) public pure returns (uint) {
		return x > 0 ? x - 1 : 0; // Underflow cannot happen
	}
}
// ====
// SMTEngine: chc
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
