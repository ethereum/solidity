pragma experimental SMTChecker;

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
		require(n > 0 && n < 100);
		b[0] = 900;
		uint[] storage a = b;
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
// Warning 6368: (321-325): CHC: Out of bounds access might happen here.
// Warning 6368: (345-349): CHC: Out of bounds access might happen here.
// Warning 6368: (338-342): CHC: Out of bounds access might happen here.
// Warning 6368: (444-448): CHC: Out of bounds access happens here.\nCounterexample:\nb = [1, 0], c = [1, 0]\nn = 1\na = []\ni = 1\n\nTransaction trace:\nLoopFor2.constructor()\nState: b = [], c = []\nLoopFor2.p()\nState: b = [0], c = [0]\nLoopFor2.p()\nState: b = [0, 0], c = [0, 0]\nLoopFor2.testUnboundedForLoop(1)
// Warning 6328: (437-456): CHC: Assertion violation happens here.\nCounterexample:\nb = [1, 0], c = [1, 0]\nn = 1\ni = 1\n\nTransaction trace:\nLoopFor2.constructor()\nState: b = [], c = []\nLoopFor2.p()\nState: b = [0], c = [0]\nLoopFor2.p()\nState: b = [0, 0], c = [0, 0]\nLoopFor2.testUnboundedForLoop(1)
// Warning 6328: (460-479): CHC: Assertion violation happens here.\nCounterexample:\nb = [1, 0], c = [1, 0]\nn = 1\ni = 1\n\nTransaction trace:\nLoopFor2.constructor()\nState: b = [], c = []\nLoopFor2.p()\nState: b = [0], c = [0]\nLoopFor2.p()\nState: b = [0, 0], c = [0, 0]\nLoopFor2.testUnboundedForLoop(1)
