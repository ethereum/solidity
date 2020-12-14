pragma experimental SMTChecker;

contract LoopFor2 {
	uint[] a;

	function testUnboundedForLoop(uint n, uint[] memory b, uint[] memory c) public {
		b[0] = 900;
		a = b;
		require(n > 0 && n < 100);
		uint i;
		while (i < n) {
			b[i] = i + 1;
			c[i] = b[i];
			++i;
		}
		// Fails due to aliasing, since both b and c are
		// memory references of same type.
		// Removed because current Spacer seg faults in cex generation.
		//assert(b[0] == c[0]);
		assert(a[0] == 900);
		// Removed because current Spacer seg faults in cex generation.
		//assert(b[0] == 900);
	}
}
// ====
// SMTSolvers: z3
// ----
// Warning 4984: (237-242): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 4984: (263-266): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
