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
// Warning 8115: (71-74): Assertion checker does not yet support the type of this variable.
// Warning 7650: (117-120): Assertion checker does not yet support this expression.
// Warning 8364: (117-118): Assertion checker does not yet implement type struct C.S storage ref
// Warning 9149: (117-125): Assertion checker does not yet implement this assignment operator.
