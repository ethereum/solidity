contract LoopFor2 {
	uint[] b;
	uint[] c;
	function p() public {
		b.push();
		c.push();
	}
	function testUnboundedForLoop(uint n) public {
		require(n < b.length);
		require(n < c.length);
		require(n > 0 && n < 10);
		//b[0] = 900; // Removed because of Spacer's nondeterminism
		uint[] storage a = b;
		uint i;
		while (i < n) {
			b[i] = i + 1;
			//c[i] = b[i]; // Removed because of Spacer's nondeterminism
			++i;
		}
		//assert(b[0] == c[0]); // Removed because of Spacer's nondeterminism
		assert(a[0] == 900);
		assert(b[0] == 900);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6368: (506-510): CHC: Out of bounds access happens here.\nCounterexample:\nb = [1, 0], c = [0, 0]\nn = 1\na = []\ni = 1\n\nTransaction trace:\nLoopFor2.constructor()\nState: b = [], c = []\nLoopFor2.p()\nState: b = [0], c = [0]\nLoopFor2.p()\nState: b = [0, 0], c = [0, 0]\nLoopFor2.testUnboundedForLoop(1)
// Warning 6328: (499-518): CHC: Assertion violation happens here.\nCounterexample:\nb = [1, 0], c = [0, 0]\nn = 1\ni = 1\n\nTransaction trace:\nLoopFor2.constructor()\nState: b = [], c = []\nLoopFor2.p()\nState: b = [0], c = [0]\nLoopFor2.p()\nState: b = [0, 0], c = [0, 0]\nLoopFor2.testUnboundedForLoop(1)
// Warning 6328: (522-541): CHC: Assertion violation happens here.\nCounterexample:\nb = [1, 0], c = [0, 0]\nn = 1\ni = 1\n\nTransaction trace:\nLoopFor2.constructor()\nState: b = [], c = []\nLoopFor2.p()\nState: b = [0], c = [0]\nLoopFor2.p()\nState: b = [0, 0], c = [0, 0]\nLoopFor2.testUnboundedForLoop(1)
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
