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
