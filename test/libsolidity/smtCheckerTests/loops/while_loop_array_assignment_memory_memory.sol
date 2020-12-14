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
		assert(a[0] == 900);
		assert(b[0] == 900);
	}
}
// ====
// SMTIgnoreCex: yes
// SMTSolvers: z3
// ----
// Warning 4984: (244-249): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 4984: (270-273): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 6328: (373-392): CHC: Assertion violation happens here.
// Warning 6328: (396-415): CHC: Assertion violation happens here.
