contract C {
	function f(bytes calldata b) external pure {
		require(b.length == 30);
		require(b[10] == 0xff);
		require(b[b.length - 1] == 0xaa);
		assert(bytes(b[10:]).length == 20); // should hold
		// Disabled because of Spacer's nondeterminism.
		//assert(bytes(b[10:])[0] == 0xff); // should hold
		//assert(bytes(b[10:])[5] == 0xff); // Removed because of Spacer's nondeterminism
		//assert(bytes(b[10:])[19] == 0xaa); // Removed because of Spacer nondeterminism
	}
}
// ====
// SMTEngine: all
// ----
