contract C {
	function f(bytes calldata b) external pure {
		require(b.length > 10);
		((b[:])[5]);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
