pragma experimental SMTChecker;
contract C {
	struct S {
		int[] b;
	}
	S s;
	struct T {
		S[] s;
	}
	T t;
	function f() public {
		s.b.push();
		t.s.push();
		t.s[0].b.push();
	}
}

// ----
// Warning 8115: (72-75): Assertion checker does not yet support the type of this variable.
// Warning 8115: (102-105): Assertion checker does not yet support the type of this variable.
// Warning 7650: (132-135): Assertion checker does not yet support this expression.
// Warning 8364: (132-133): Assertion checker does not yet implement type struct C.S storage ref
// Warning 9599: (132-135): Assertion checker does not yet implement this expression.
// Warning 7650: (146-149): Assertion checker does not yet support this expression.
// Warning 8364: (146-147): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (146-156): Assertion checker does not yet implement type struct C.S storage ref
// Warning 9599: (146-149): Assertion checker does not yet implement this expression.
// Warning 7650: (160-168): Assertion checker does not yet support this expression.
// Warning 7650: (160-163): Assertion checker does not yet support this expression.
// Warning 8364: (160-161): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (160-166): Assertion checker does not yet implement type struct C.S storage ref
// Warning 9118: (160-166): Assertion checker does not yet implement this expression.
// Warning 9599: (160-168): Assertion checker does not yet implement this expression.
