pragma experimental SMTChecker;
contract C {
	int[1] c;
	function f(bool b) public {
		if (b)
			c[0] |= 1;
	}
}
// ----
// Warning 9149: (97-106): Assertion checker does not yet implement this assignment operator.
