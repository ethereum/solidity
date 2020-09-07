pragma experimental SMTChecker;
contract C {
	struct S {
		uint x;
	}
	S s;
	function f(bool b) public {
		if (b)
			s.x |= 1;
	}
}
// ----
// Warning 9149: (117-125): Assertion checker does not yet implement this assignment operator.
