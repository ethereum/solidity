contract LoopFor2 {
	function testUnboundedForLoop(uint n, uint[] memory b, uint[] memory c) public pure {
		require(n < b.length);
		require(n < c.length);
		require(n > 0 && n < 100);
		b[0] = 900;
		uint[] memory a = b;
		uint i;
		// Disabled because of Spacer's nondeterminism.
		/*
		while (i < n) {
			// Accesses are safe but oob is reported due to potential aliasing after c's assignment.
			b[i] = i + 1;
			c[i] = b[i];
			++i;
		}
		*/
		// Removed because current Spacer seg faults in cex generation.
		//assert(b[0] == c[0]);
		// Removed because current Spacer seg faults in cex generation.
		//assert(a[0] == 900);
		// Removed because current Spacer seg faults in cex generation.
		//assert(b[0] == 900);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// SMTSolvers: z3
// ----
// Warning 2072: (202-217): Unused local variable.
// Warning 2072: (225-231): Unused local variable.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
