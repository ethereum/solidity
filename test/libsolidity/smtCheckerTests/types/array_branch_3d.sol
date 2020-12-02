pragma experimental SMTChecker;

contract C
{
	uint[][][] c;
	function f(bool b) public {
		c[0][0][0] = 0;
		if (b)
			c[0][0][0] = 1;
		assert(c[0][0][0] > 0);
	}
}
// ----
// Warning 6328: (138-160): CHC: Assertion violation happens here.\nCounterexample:\nc = []\nb = false\n\n\nTransaction trace:\nconstructor()\nState: c = []\nf(false)
