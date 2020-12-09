pragma experimental SMTChecker;

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
// ----
// Warning 6328: (317-343): CHC: Assertion violation happens here.\nCounterexample:\nb = [[0], [0]]\n\n\n\nTransaction trace:\nconstructor()\nState: b = []\nf()
