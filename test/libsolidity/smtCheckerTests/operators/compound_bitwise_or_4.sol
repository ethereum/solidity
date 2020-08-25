pragma experimental SMTChecker;
contract C {
	struct S {
		uint[] x;
	}
	S s;
	function f(bool b) public {
		if (b)
			s.x[2] |= 1;
	}
}
// ----
// Warning 9149: (119-130): Assertion checker does not yet implement this assignment operator.
