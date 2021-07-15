contract LoopFor2 {
	uint[] a;
	function p() public {
		a.push();
	}
	function testUnboundedForLoop(uint n, uint[] memory b, uint[] memory c) public {
		require(n < a.length);
		require(n < b.length);
		require(n < c.length);
		b[0] = 900;
		a = b;
		require(n > 0 && n < 100);
		uint i;
		while (i < n) {
			// Accesses are safe but oob is reported due to potential aliasing after c's assignment.
			b[i] = i + 1;
			c[i] = b[i];
			++i;
		}
		// Fails due to aliasing, since both b and c are
		// memory references of same type.
		assert(b[0] == c[0]);
		assert(a[0] == 900);
		assert(b[0] == 900);
	}
}
// ====
// SMTEngine: all
// SMTSolvers: z3
// ----
// Warning 6368: (401-405): CHC: Out of bounds access happens here.
// Warning 6368: (425-429): CHC: Out of bounds access happens here.
// Warning 1218: (418-422): CHC: Error trying to invoke SMT solver.
// Warning 6368: (418-422): CHC: Out of bounds access might happen here.
// Warning 6368: (540-544): CHC: Out of bounds access happens here.
// Warning 6368: (548-552): CHC: Out of bounds access happens here.
// Warning 6328: (533-553): CHC: Assertion violation happens here.
// Warning 6368: (587-591): CHC: Out of bounds access happens here.
// Warning 6328: (580-599): CHC: Assertion violation happens here.
