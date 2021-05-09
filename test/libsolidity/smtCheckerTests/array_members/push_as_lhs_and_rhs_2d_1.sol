contract C {
	uint[][] b;
	function f() public {
		require(b.length == 0);
		b.push().push() = b.push().push();
		assert(b.length == 2);
		assert(b[0].length == 1);
		assert(b[0].length == 1);
		assert(b[0][0] == 0);
		assert(b[1][0] == 0);
		assert(b[0][0] == b[1][0]);
		// Fails
		assert(b[0][0] != b[1][0]);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (284-310): CHC: Assertion violation happens here.\nCounterexample:\nb = [[0], [0]]\n\nTransaction trace:\nC.constructor()\nState: b = []\nC.f()
