pragma experimental SMTChecker;

contract C {
	uint[][] c;

	function f() public {
		require(c.length == 0);
		c.push().push() = 2;
		assert(c.length == 1);
		assert(c[0].length == 1);
		assert(c[0][0] == 2);
	}

	function g() public {
		c.push().push() = 2;
		assert(c.length > 0);
		assert(c[c.length - 1].length == 1);
		assert(c[c.length - 1][c[c.length - 1].length - 1] == 2);
		// Fails
		assert(c[c.length - 1][c[c.length - 1].length - 1] == 200);
	}
}
// ----
// Warning 6328: (395-453): CHC: Assertion violation happens here.\nCounterexample:\nc = [[2]]\n\n\n\nTransaction trace:\nconstructor()\nState: c = []\ng()
