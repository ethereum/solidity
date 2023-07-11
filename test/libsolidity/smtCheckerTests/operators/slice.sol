contract C {
	function f(bytes calldata b) external pure {
		require(b.length > 10);
		require(b[10] == 0xff);
		assert(bytes(b[10:20]).length == 10);
		// Disabled because of Spacer nondeterminism
		//assert(bytes(b[10:20])[0] == 0xff);
		// Disabled because of Spacer nondeterminism
		//assert(bytes(b[10:20])[5] == 0xff);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
