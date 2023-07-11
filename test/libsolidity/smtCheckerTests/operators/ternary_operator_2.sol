contract C {

	function decrement(uint x) private pure returns (uint) {
		return x - 1; // No underflow, the method can be called only with positive value
	}

	function f(uint x) public pure returns (uint) {
		return x > 0 ? decrement(x) : 0;
	}
}
// ====
// SMTEngine: chc
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
