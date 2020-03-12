pragma experimental SMTChecker;

contract LoopFor2 {
	function testUnboundedForLoop(uint[] memory b, uint[] memory c) public pure {
		b[0] = 900;
		uint[] memory a = b;
		require(b.length == c.length);
		require(b.length > 0 && b.length < 100);
		for (uint i = 0; i < b.length; i++) {
			b[i] = i + 1;
			c[i] = b[i];
		}
		assert(b[0] == c[0]);
		assert(a[0] == 900);
		assert(b[0] == 900);
	}
}
// ----
// Warning: (295-300): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (324-344): Assertion violation happens here
// Warning: (348-367): Assertion violation happens here
// Warning: (371-390): Assertion violation happens here
