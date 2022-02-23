contract C {
	function f(bytes calldata b) external pure {
		require(b.length == 30);
		require(b[10] == 0xff);
		require(b[b.length - 1] == 0xaa);
		assert(bytes(b[10:]).length == 20);
		assert(bytes(b[10:])[0] == 0xff);
		//assert(bytes(b[10:])[5] == 0xff); // Removed because of Spacer's nondeterminism
		//assert(bytes(b[10:])[19] == 0xaa); // Removed because of Spacer nondeterminism
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (188-220): CHC: Assertion violation might happen here.
// Warning 4661: (188-220): BMC: Assertion violation happens here.
