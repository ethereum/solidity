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
// ====
// SMTEngine: all
// ----
// Warning 8115: (48-52): Assertion checker does not yet support the type of this variable.
// Warning 8115: (55-59): Assertion checker does not yet support the type of this variable.
// Warning 7650: (98-102): Assertion checker does not yet support this expression.
// Warning 8364: (98-100): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (106-110): Assertion checker does not yet support this expression.
// Warning 8364: (106-108): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (122-126): Assertion checker does not yet support this expression.
// Warning 8364: (122-124): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (137-141): Assertion checker does not yet support this expression.
// Warning 8364: (137-139): Assertion checker does not yet implement type struct C.S storage ref
// Warning 6328: (91-111): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (115-149): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
