pragma experimental SMTChecker;

contract C {
	struct S {
		uint x;
		S[] a;
	}
	S s1;
	S s2;
	function f() public view {
		assert(s1.x == s2.x);
		assert(s1.a.length == s2.a.length);
	}
}
// ----
// Warning 8115: (81-85): Assertion checker does not yet support the type of this variable.
// Warning 8115: (88-92): Assertion checker does not yet support the type of this variable.
// Warning 7650: (131-135): Assertion checker does not yet support this expression.
// Warning 8364: (131-133): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (139-143): Assertion checker does not yet support this expression.
// Warning 8364: (139-141): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (155-159): Assertion checker does not yet support this expression.
// Warning 8364: (155-157): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (170-174): Assertion checker does not yet support this expression.
// Warning 8364: (170-172): Assertion checker does not yet implement type struct C.S storage ref
// Warning 6328: (124-144): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (148-182): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 8115: (81-85): Assertion checker does not yet support the type of this variable.
// Warning 8115: (88-92): Assertion checker does not yet support the type of this variable.
// Warning 7650: (131-135): Assertion checker does not yet support this expression.
// Warning 8364: (131-133): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (139-143): Assertion checker does not yet support this expression.
// Warning 8364: (139-141): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (155-159): Assertion checker does not yet support this expression.
// Warning 8364: (155-157): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (170-174): Assertion checker does not yet support this expression.
// Warning 8364: (170-172): Assertion checker does not yet implement type struct C.S storage ref
