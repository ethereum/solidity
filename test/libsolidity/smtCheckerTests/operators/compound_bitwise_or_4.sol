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
// Warning 8115: (73-76): Assertion checker does not yet support the type of this variable.
// Warning 7650: (119-122): Assertion checker does not yet support this expression.
// Warning 8364: (119-120): Assertion checker does not yet implement type struct C.S storage ref
// Warning 9118: (119-125): Assertion checker does not yet implement this expression.
// Warning 9149: (119-130): Assertion checker does not yet implement this assignment operator.
