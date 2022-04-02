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
// Warning 8115: (48-52='S s1'): Assertion checker does not yet support the type of this variable.
// Warning 8115: (55-59='S s2'): Assertion checker does not yet support the type of this variable.
// Warning 7650: (98-102='s1.x'): Assertion checker does not yet support this expression.
// Warning 8364: (98-100='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (106-110='s2.x'): Assertion checker does not yet support this expression.
// Warning 8364: (106-108='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (122-126='s1.a'): Assertion checker does not yet support this expression.
// Warning 8364: (122-124='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (137-141='s2.a'): Assertion checker does not yet support this expression.
// Warning 8364: (137-139='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (160-169='s1.a[0].x'): Assertion checker does not yet support this expression.
// Warning 7650: (160-164='s1.a'): Assertion checker does not yet support this expression.
// Warning 8364: (160-162='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (160-167='s1.a[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (173-182='s2.a[0].x'): Assertion checker does not yet support this expression.
// Warning 7650: (173-177='s2.a'): Assertion checker does not yet support this expression.
// Warning 8364: (173-175='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (173-180='s2.a[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (213-217='s1.x'): Assertion checker does not yet support this expression.
// Warning 8364: (213-215='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (213-217='s1.x'): Assertion checker does not support recursive structs.
// Warning 7650: (226-230='s2.x'): Assertion checker does not yet support this expression.
// Warning 8364: (226-228='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (226-230='s2.x'): Assertion checker does not support recursive structs.
// Warning 7650: (239-243='s1.a'): Assertion checker does not yet support this expression.
// Warning 8364: (239-241='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (239-250='s1.a.push()'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (239-243='s1.a'): Assertion checker does not support recursive structs.
// Warning 7650: (254-258='s2.a'): Assertion checker does not yet support this expression.
// Warning 8364: (254-256='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (254-265='s2.a.push()'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (254-258='s2.a'): Assertion checker does not support recursive structs.
// Warning 7650: (269-278='s1.a[0].x'): Assertion checker does not yet support this expression.
// Warning 7650: (269-273='s1.a'): Assertion checker does not yet support this expression.
// Warning 8364: (269-271='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (269-276='s1.a[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (269-278='s1.a[0].x'): Assertion checker does not support recursive structs.
// Warning 7650: (287-296='s2.a[0].x'): Assertion checker does not yet support this expression.
// Warning 7650: (287-291='s2.a'): Assertion checker does not yet support this expression.
// Warning 8364: (287-289='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (287-294='s2.a[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (287-296='s2.a[0].x'): Assertion checker does not support recursive structs.
// Warning 6328: (91-111='assert(s1.x == s2.x)'): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (115-149): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (160-167='s1.a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (173-180='s2.a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (153-183='assert(s1.a[0].x == s2.a[0].x)'): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (269-276='s1.a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (287-294='s2.a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
