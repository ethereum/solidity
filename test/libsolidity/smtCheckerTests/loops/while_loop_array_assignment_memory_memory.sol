pragma experimental SMTChecker;

contract LoopFor2 {
	function testUnboundedForLoop(uint n, uint[] memory b, uint[] memory c) public pure {
		b[0] = 900;
		uint[] memory a = b;
		require(n > 0 && n < 100);
		uint i;
		while (i < n) {
			b[i] = i + 1;
			c[i] = b[i];
			++i;
		}
		// Removed because current Spacer seg faults in cex generation.
		//assert(b[0] == c[0]);
		// Removed because current Spacer seg faults in cex generation.
		//assert(a[0] == 900);
		// Removed because current Spacer seg faults in cex generation.
		//assert(b[0] == 900);
	}
}
// ====
// SMTIgnoreCex: yes
// SMTSolvers: z3
// ----
// Warning 2072: (156-171): Unused local variable.
