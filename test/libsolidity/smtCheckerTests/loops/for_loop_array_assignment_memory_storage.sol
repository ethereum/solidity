pragma experimental SMTChecker;

contract LoopFor2 {
	uint[] a;

	function testUnboundedForLoop(uint n, uint[] memory b, uint[] memory c) public {
		b[0] = 900;
		a = b;
		require(n > 0 && n < 100);
		for (uint i = 0; i < n; i += 1) {
			b[i] = i + 1;
			c[i] = b[i];
		}
		// Removed because current Spacer seg faults in cex generation.
		//assert(b[0] == c[0]);
		//assert(a[0] == 900);
		//assert(b[0] == 900);
	}
}
// ====
// SMTSolvers: z3
// ----
// Warning 4984: (245-250): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 4984: (225-231): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
