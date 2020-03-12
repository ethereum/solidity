pragma experimental SMTChecker;

contract LoopFor2 {
	uint[] b;
	uint[] c;

	function testUnboundedForLoop() public {
		b[0] = 900;
		uint[] storage a = b;
		require(b.length == c.length);
		require(b.length > 0 && b.length < 100);
		for (uint i = 0; i < b.length; i++) {
			b[i] = i + 1;
			c[i] = b[i];
		}
		// This is safe but too hard to prove currently.
		assert(b[0] == c[0]);
		assert(a[0] == 900);
		assert(b[0] == 900);
	}
}
// ----
// Warning: (282-287): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (362-382): Assertion violation happens here
// Warning: (386-405): Assertion violation happens here
// Warning: (409-428): Assertion violation happens here
