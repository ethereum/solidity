pragma experimental SMTChecker;

contract LoopFor2 {
	uint[] b;
	uint[] c;

	function testUnboundedForLoop(uint n) public {
		b[0] = 900;
		uint[] storage a = b;
		require(n > 0 && n < 100);
		for (uint i = 0; i < n; i += 1) {
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
// Warning 4984: (237-242): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 4984: (217-223): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 6328: (341-360): CHC: Assertion violation happens here.\nCounterexample:\nb = [], c = []\nn = 1\n\n\nTransaction trace:\nconstructor()\nState: b = [], c = []\ntestUnboundedForLoop(1)
// Warning 6328: (364-383): CHC: Assertion violation happens here.\nCounterexample:\nb = [], c = []\nn = 1\n\n\nTransaction trace:\nconstructor()\nState: b = [], c = []\ntestUnboundedForLoop(1)
