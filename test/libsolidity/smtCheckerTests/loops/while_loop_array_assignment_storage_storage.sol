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
// Warning 6328: (338-357): CHC: Assertion violation happens here.\nCounterexample:\nb = [], c = []\nn = 1\n\n\nTransaction trace:\nLoopFor2.constructor()\nState: b = [], c = []\nLoopFor2.testUnboundedForLoop(1)
// Warning 6328: (361-380): CHC: Assertion violation happens here.\nCounterexample:\nb = [], c = []\nn = 1\n\n\nTransaction trace:\nLoopFor2.constructor()\nState: b = [], c = []\nLoopFor2.testUnboundedForLoop(1)
