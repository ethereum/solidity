contract LoopFor2 {
	uint[] b;
	uint[] c;

	// Disabled because of Spancer nondeterminism in the overflow queries
	// which can't be disabled separately.
	/*
	function testUnboundedForLoop(uint n) public {
		b[0] = 900;
		uint[] storage a = b;
		require(n > 0 && n < 100);
		for (uint i = 0; i < n; i += 1) {
			b[i] = i + 1;
			c[i] = b[i];
		}
		// This is safe but too hard to prove currently.
		assert(b[0] == c[0]);
		assert(a[0] == 900);
		assert(b[0] == 900);
	}
	*/
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
