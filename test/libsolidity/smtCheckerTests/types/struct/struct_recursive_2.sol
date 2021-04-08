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
		assert(s1.a[0].x == s2.a[0].x);
	}
	function g() public {
		s1.x = 42;
		s2.x = 42;
		s1.a.push();
		s2.a.push();
		s1.a[0].x = 43;
		s2.a[0].x = 43;
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
// Warning 7650: (160-169): Assertion checker does not yet support this expression.
// Warning 7650: (160-164): Assertion checker does not yet support this expression.
// Warning 8364: (160-162): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (160-167): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (173-182): Assertion checker does not yet support this expression.
// Warning 7650: (173-177): Assertion checker does not yet support this expression.
// Warning 8364: (173-175): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (173-180): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (213-217): Assertion checker does not yet support this expression.
// Warning 8364: (213-215): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (213-217): Assertion checker does not support recursive structs.
// Warning 7650: (226-230): Assertion checker does not yet support this expression.
// Warning 8364: (226-228): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (226-230): Assertion checker does not support recursive structs.
// Warning 7650: (239-243): Assertion checker does not yet support this expression.
// Warning 8364: (239-241): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (239-250): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (239-243): Assertion checker does not support recursive structs.
// Warning 7650: (254-258): Assertion checker does not yet support this expression.
// Warning 8364: (254-256): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (254-265): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (254-258): Assertion checker does not support recursive structs.
// Warning 7650: (269-278): Assertion checker does not yet support this expression.
// Warning 7650: (269-273): Assertion checker does not yet support this expression.
// Warning 8364: (269-271): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (269-276): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (269-278): Assertion checker does not support recursive structs.
// Warning 7650: (287-296): Assertion checker does not yet support this expression.
// Warning 7650: (287-291): Assertion checker does not yet support this expression.
// Warning 8364: (287-289): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (287-294): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (287-296): Assertion checker does not support recursive structs.
// Warning 6328: (91-111): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (115-149): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (160-167): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (173-180): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (153-183): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (269-276): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (287-294): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
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
// Warning 7650: (160-169): Assertion checker does not yet support this expression.
// Warning 7650: (160-164): Assertion checker does not yet support this expression.
// Warning 8364: (160-162): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (160-167): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (173-182): Assertion checker does not yet support this expression.
// Warning 7650: (173-177): Assertion checker does not yet support this expression.
// Warning 8364: (173-175): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (173-180): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (213-217): Assertion checker does not yet support this expression.
// Warning 8364: (213-215): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (213-217): Assertion checker does not support recursive structs.
// Warning 7650: (226-230): Assertion checker does not yet support this expression.
// Warning 8364: (226-228): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (226-230): Assertion checker does not support recursive structs.
// Warning 7650: (239-243): Assertion checker does not yet support this expression.
// Warning 8364: (239-241): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (239-250): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (239-243): Assertion checker does not support recursive structs.
// Warning 7650: (254-258): Assertion checker does not yet support this expression.
// Warning 8364: (254-256): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (254-265): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (254-258): Assertion checker does not support recursive structs.
// Warning 7650: (269-278): Assertion checker does not yet support this expression.
// Warning 7650: (269-273): Assertion checker does not yet support this expression.
// Warning 8364: (269-271): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (269-276): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (269-278): Assertion checker does not support recursive structs.
// Warning 7650: (287-296): Assertion checker does not yet support this expression.
// Warning 7650: (287-291): Assertion checker does not yet support this expression.
// Warning 8364: (287-289): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (287-294): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (287-296): Assertion checker does not support recursive structs.
