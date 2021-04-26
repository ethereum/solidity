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
// Warning 8115: (82-86): Assertion checker does not yet support the type of this variable.
// Warning 8115: (89-93): Assertion checker does not yet support the type of this variable.
// Warning 7650: (132-136): Assertion checker does not yet support this expression.
// Warning 8364: (132-134): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (140-144): Assertion checker does not yet support this expression.
// Warning 8364: (140-142): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (156-160): Assertion checker does not yet support this expression.
// Warning 8364: (156-158): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (171-175): Assertion checker does not yet support this expression.
// Warning 8364: (171-173): Assertion checker does not yet implement type struct C.S storage ref
// Warning 6328: (125-145): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (149-183): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 8115: (82-86): Assertion checker does not yet support the type of this variable.
// Warning 8115: (89-93): Assertion checker does not yet support the type of this variable.
// Warning 7650: (132-136): Assertion checker does not yet support this expression.
// Warning 8364: (132-134): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (140-144): Assertion checker does not yet support this expression.
// Warning 8364: (140-142): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (156-160): Assertion checker does not yet support this expression.
// Warning 8364: (156-158): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (171-175): Assertion checker does not yet support this expression.
// Warning 8364: (171-173): Assertion checker does not yet implement type struct C.S storage ref
