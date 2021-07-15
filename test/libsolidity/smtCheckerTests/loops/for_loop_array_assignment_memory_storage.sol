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
			c[i] = b[i];
		}
		assert(b[0] == c[0]); // should fail due to aliasing
		assert(a[0] == 900); // should hold
		assert(b[0] == 900); // should fail
	}
}
// ====
// SMTEngine: all
// SMTSolvers: z3
// ----
// Warning 6368: (409-413): CHC: Out of bounds access happens here.
// Warning 6368: (433-437): CHC: Out of bounds access happens here.
// Warning 1218: (426-430): CHC: Error trying to invoke SMT solver.
// Warning 6368: (426-430): CHC: Out of bounds access might happen here.
// Warning 1218: (452-456): CHC: Error trying to invoke SMT solver.
// Warning 6368: (452-456): CHC: Out of bounds access might happen here.
// Warning 1218: (460-464): CHC: Error trying to invoke SMT solver.
// Warning 6368: (460-464): CHC: Out of bounds access might happen here.
// Warning 6328: (445-465): CHC: Assertion violation happens here.
// Warning 6368: (545-549): CHC: Out of bounds access happens here.
// Warning 1218: (538-557): CHC: Error trying to invoke SMT solver.
// Warning 6328: (538-557): CHC: Assertion violation might happen here.
// Warning 4661: (538-557): BMC: Assertion violation happens here.
