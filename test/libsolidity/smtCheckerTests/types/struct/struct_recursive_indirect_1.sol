pragma experimental SMTChecker;

contract C {
	struct S {
		uint x;
		T[] a;
	}
	struct T {
		uint y;
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
// Warning 8115: (115-119): Assertion checker does not yet support the type of this variable.
// Warning 8115: (122-126): Assertion checker does not yet support the type of this variable.
// Warning 7650: (165-169): Assertion checker does not yet support this expression.
// Warning 8364: (165-167): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (173-177): Assertion checker does not yet support this expression.
// Warning 8364: (173-175): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (189-193): Assertion checker does not yet support this expression.
// Warning 8364: (189-191): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (204-208): Assertion checker does not yet support this expression.
// Warning 8364: (204-206): Assertion checker does not yet implement type struct C.S storage ref
// Warning 6328: (158-178): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (182-216): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 8115: (115-119): Assertion checker does not yet support the type of this variable.
// Warning 8115: (122-126): Assertion checker does not yet support the type of this variable.
// Warning 7650: (165-169): Assertion checker does not yet support this expression.
// Warning 8364: (165-167): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (173-177): Assertion checker does not yet support this expression.
// Warning 8364: (173-175): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (189-193): Assertion checker does not yet support this expression.
// Warning 8364: (189-191): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (204-208): Assertion checker does not yet support this expression.
// Warning 8364: (204-206): Assertion checker does not yet implement type struct C.S storage ref
