pragma experimental SMTChecker;

contract LoopFor2 {
	function testUnboundedForLoop(uint[] memory b, uint[] memory c) public pure {
		b[0] = 900;
		uint[] memory a = b;
		require(b.length == c.length);
		require(b.length > 0 && b.length < 100);
		uint i;
		while (i < b.length) {
			b[i] = i + 1;
			c[i] = b[i];
			++i;
		}
		assert(b[0] == c[0]);
		assert(a[0] == 900);
		assert(b[0] == 900);
	}
}
// ----
// Warning: (290-295): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (316-319): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (327-347): Assertion violation happens here
// Warning: (351-370): Assertion violation happens here
// Warning: (374-393): Assertion violation happens here
