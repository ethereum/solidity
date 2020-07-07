pragma experimental SMTChecker;
contract C {
	struct S {
		int[] b;
	}
	S s;
	struct T {
		S s;
	}
	T t;
	function f() public {
		s.b.push();
		t.s.b.push();
	}
}

// ----
// Warning 8115: (72-75): Assertion checker does not yet support the type of this variable.
// Warning 8115: (100-103): Assertion checker does not yet support the type of this variable.
// Warning 7650: (130-133): Assertion checker does not yet support this expression.
// Warning 8364: (130-131): Assertion checker does not yet implement type struct C.S storage ref
// Warning 9599: (130-133): Assertion checker does not yet implement this expression.
// Warning 7650: (144-149): Assertion checker does not yet support this expression.
// Warning 8364: (144-147): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (144-147): Assertion checker does not yet support this expression.
// Warning 8364: (144-145): Assertion checker does not yet implement type struct C.T storage ref
// Warning 9599: (144-149): Assertion checker does not yet implement this expression.
