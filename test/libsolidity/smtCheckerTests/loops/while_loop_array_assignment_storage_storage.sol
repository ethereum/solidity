pragma experimental SMTChecker;

contract LoopFor2 {
	uint[] b;
	uint[] c;

	function testUnboundedForLoop() public {
		b[0] = 900;
		uint[] storage a = b;
		require(b.length == c.length);
		require(b.length > 0 && b.length < 100);
		uint i;
		while (i < b.length) {
			b[i] = i + 1;
			c[i] = b[i];
			++i;
		}
		// Fails as false positive.
		assert(b[0] == c[0]);
		assert(a[0] == 900);
		assert(b[0] == 900);
	}
}
// ----
// Warning: (277-282): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (303-306): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (344-364): Assertion violation happens here
// Warning: (368-387): Assertion violation happens here
// Warning: (391-410): Assertion violation happens here
