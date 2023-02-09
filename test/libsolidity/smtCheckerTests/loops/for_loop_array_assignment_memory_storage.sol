contract LoopFor2 {
	uint[] a;
	function p() public {
		a.push();
	}
	function testUnboundedForLoop(uint n, uint[] memory b, uint[] memory c) public {
		require(n < a.length);
		require(n < b.length);
		require(n < c.length);
		require(n > 0 && n < 100);
		b[0] = 900;
		a = b;
		for (uint i = 0; i < n; i += 1) {
			// Accesses are safe but oob is reported due to potential aliasing after c's assignment.
			b[i] = i + 1;
			// Disabled because of Spacer's nondeterminism.
			//c[i] = b[i];
		}
		// Removed because current Spacer seg faults in cex generation.
		//assert(b[0] == c[0]);
		//assert(a[0] == 900);
		//assert(b[0] == 900);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// SMTSolvers: z3
// ----
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
