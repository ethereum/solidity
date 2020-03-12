pragma experimental SMTChecker;

contract LoopFor2 {
	uint[] a;

	function testUnboundedForLoop(uint[] memory b, uint[] memory c) public {
		b[0] = 900;
		a = b;
		require(b.length == c.length);
		require(b.length > 0 && b.length < 100);
		uint i;
		while (i < b.length) {
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
// Warning: (283-288): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (309-312): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (408-428): Assertion violation happens here
// Warning: (455-474): Assertion violation happens here
