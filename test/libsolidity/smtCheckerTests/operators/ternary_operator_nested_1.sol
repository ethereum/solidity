contract C {

	function unreachable() private pure returns (uint) {
		assert(false);
		return 0;
	}

	function f(uint x) public pure returns (uint) {
		return x <= 1 ? 0 : x < 2 ? unreachable() : 0;
	}
}
// ====
// SMTEngine: chc
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
