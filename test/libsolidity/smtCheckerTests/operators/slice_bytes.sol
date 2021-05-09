contract C {
	function f(bytes calldata b) external pure {
		require(b.length > 10);
		((b[:])[5]);
	}
}
// ====
// SMTEngine: all
// ----
