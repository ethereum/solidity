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
		assert(b[0] == c[0]);
		assert(a[0] == 900);
		assert(b[0] == 900);
	}
}
// ----
// Warning: (362-382): Assertion violation happens here
// Warning: (409-428): Assertion violation happens here
