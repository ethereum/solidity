pragma experimental SMTChecker;
contract C {
	int[1][20] c;
	function f(bool b) public {
		if (b)
			c[10][0] |= 1;
	}
}
// ----
// Warning 9149: (101-114): Assertion checker does not yet implement this assignment operator.
