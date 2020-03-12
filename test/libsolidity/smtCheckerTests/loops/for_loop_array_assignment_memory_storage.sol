pragma experimental SMTChecker;

contract LoopFor2 {
	uint[] a;

	function testUnboundedForLoop(uint[] memory b, uint[] memory c) public {
		b[0] = 900;
		a = b;
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
// Warning: (288-293): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (317-337): Assertion violation happens here
// Warning: (364-383): Assertion violation happens here
