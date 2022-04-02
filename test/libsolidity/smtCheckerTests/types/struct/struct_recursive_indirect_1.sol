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
// ====
// SMTEngine: all
// ----
// Warning 8115: (82-86='S s1'): Assertion checker does not yet support the type of this variable.
// Warning 8115: (89-93='S s2'): Assertion checker does not yet support the type of this variable.
// Warning 7650: (132-136='s1.x'): Assertion checker does not yet support this expression.
// Warning 8364: (132-134='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (140-144='s2.x'): Assertion checker does not yet support this expression.
// Warning 8364: (140-142='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (156-160='s1.a'): Assertion checker does not yet support this expression.
// Warning 8364: (156-158='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (171-175='s2.a'): Assertion checker does not yet support this expression.
// Warning 8364: (171-173='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 6328: (125-145='assert(s1.x == s2.x)'): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (149-183): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
