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
// Warning: (72-75): Assertion checker does not yet support the type of this variable.
// Warning: (100-103): Assertion checker does not yet support the type of this variable.
// Warning: (130-133): Assertion checker does not yet support this expression.
// Warning: (130-131): Assertion checker does not yet implement type struct C.S storage ref
// Warning: (130-133): Assertion checker does not yet implement this expression.
// Warning: (144-149): Assertion checker does not yet support this expression.
// Warning: (144-147): Assertion checker does not yet implement type struct C.S storage ref
// Warning: (144-147): Assertion checker does not yet support this expression.
// Warning: (144-145): Assertion checker does not yet implement type struct C.T storage ref
// Warning: (144-149): Assertion checker does not yet implement this expression.
