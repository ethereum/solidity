pragma experimental SMTChecker;

contract LoopFor2 {
	uint[] b;
	uint[] c;

	function testUnboundedForLoop(uint n) public {
		b[0] = 900;
		uint[] memory a = b;
		require(b.length == c.length);
		require(b.length > 0 && b.length < 100);
		for (uint i = 0; i < b.length; i++) {
			b[i] = i + 1;
			c[i] = b[i];
		}
		// This is safe but too hard to solve currently.
		assert(b[0] == c[0]);
		assert(a[0] == 900);
		assert(b[0] == 900);
	}
}
// ----
// Warning: (107-113): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (287-292): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (367-387): Assertion violation happens here
// Warning: (414-433): Assertion violation happens here
