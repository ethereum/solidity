pragma experimental SMTChecker;

contract C
{
	function f(bool b, uint[] memory c) public {
		c[0] = 0;
		if (b)
			c[0] = 1;
		assert(c[0] > 0);
	}
}
// ----
// Warning 2018: (47-148): Function state mutability can be restricted to pure
// Warning 6328: (128-144): CHC: Assertion violation happens here.\nCounterexample:\n\nb = false\nc = [0, 5, 5, 5, 5, 5]\n\n\nTransaction trace:\nconstructor()\nf(false, [38, 5, 5, 5, 5, 5])
