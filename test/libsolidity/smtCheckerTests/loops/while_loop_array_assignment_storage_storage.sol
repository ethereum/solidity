pragma experimental SMTChecker;

contract LoopFor2 {
	uint[] b;
	uint[] c;

	function testUnboundedForLoop(uint n) public {
		b[0] = 900;
		uint[] storage a = b;
		require(n > 0 && n < 100);
		uint i;
		while (i < n) {
			b[i] = i + 1;
			c[i] = b[i];
			++i;
		}
		//assert(b[0] == c[0]); // Removed because of Spacer's nondeterminism
		assert(a[0] == 900);
		assert(b[0] == 900);
	}
}
// ----
// Warning 4984: (229-234): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 4984: (255-258): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 6328: (338-357): CHC: Assertion violation happens here.\nCounterexample:\nb = [], c = []\nn = 1\n\n\nTransaction trace:\nconstructor()\nState: b = [], c = []\ntestUnboundedForLoop(1)
// Warning 6328: (361-380): CHC: Assertion violation happens here.\nCounterexample:\nb = [], c = []\nn = 1\n\n\nTransaction trace:\nconstructor()\nState: b = [], c = []\ntestUnboundedForLoop(1)
