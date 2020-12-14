pragma experimental SMTChecker;

contract C {
	bytes[] c;

	function f() public {
		bytes1 val = bytes1(uint8(2));
		require(c.length == 0);
		c.push().push() = val;
		assert(c.length == 1);
		assert(c[0].length == 1);
		assert(c[0][0] == val);
	}

	function g() public {
		bytes1 val = bytes1(uint8(2));
		c.push().push() = val;
		assert(c.length > 0);
		assert(c[c.length - 1].length == 1);
		assert(c[c.length - 1][c[c.length - 1].length - 1] == val);
		// Fails
		assert(c[c.length - 1][c[c.length - 1].length - 1] == bytes1(uint8(100)));
	}
}
// ----
// Warning 6328: (468-541): CHC: Assertion violation happens here.\nCounterexample:\nc = [[2]]\n\n\n\nTransaction trace:\nconstructor()\nState: c = []\ng()
