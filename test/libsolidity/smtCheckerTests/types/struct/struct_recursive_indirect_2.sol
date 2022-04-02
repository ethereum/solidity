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
	function f() public {
		s1.a.push();
		s2.a.push();
		s1.a[0].a.push();
		s2.a[0].a.push();
		assert(s1.a[0].a[0].x == s2.a[0].a[0].x);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 8115: (82-86='S s1'): Assertion checker does not yet support the type of this variable.
// Warning 8115: (89-93='S s2'): Assertion checker does not yet support the type of this variable.
// Warning 7650: (120-124='s1.a'): Assertion checker does not yet support this expression.
// Warning 8364: (120-122='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (120-131='s1.a.push()'): Assertion checker does not yet implement type struct C.T storage ref
// Warning 4375: (120-124='s1.a'): Assertion checker does not support recursive structs.
// Warning 7650: (135-139='s2.a'): Assertion checker does not yet support this expression.
// Warning 8364: (135-137='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (135-146='s2.a.push()'): Assertion checker does not yet implement type struct C.T storage ref
// Warning 4375: (135-139='s2.a'): Assertion checker does not support recursive structs.
// Warning 7650: (150-159='s1.a[0].a'): Assertion checker does not yet support this expression.
// Warning 7650: (150-154='s1.a'): Assertion checker does not yet support this expression.
// Warning 8364: (150-152='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (150-157='s1.a[0]'): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (150-166='s1.a[0].a.push()'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (150-159='s1.a[0].a'): Assertion checker does not support recursive structs.
// Warning 7650: (170-179='s2.a[0].a'): Assertion checker does not yet support this expression.
// Warning 7650: (170-174='s2.a'): Assertion checker does not yet support this expression.
// Warning 8364: (170-172='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (170-177='s2.a[0]'): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (170-186='s2.a[0].a.push()'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (170-179='s2.a[0].a'): Assertion checker does not support recursive structs.
// Warning 7650: (197-211='s1.a[0].a[0].x'): Assertion checker does not yet support this expression.
// Warning 7650: (197-206='s1.a[0].a'): Assertion checker does not yet support this expression.
// Warning 7650: (197-201='s1.a'): Assertion checker does not yet support this expression.
// Warning 8364: (197-199='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (197-204='s1.a[0]'): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (197-209='s1.a[0].a[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (215-229='s2.a[0].a[0].x'): Assertion checker does not yet support this expression.
// Warning 7650: (215-224='s2.a[0].a'): Assertion checker does not yet support this expression.
// Warning 7650: (215-219='s2.a'): Assertion checker does not yet support this expression.
// Warning 8364: (215-217='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (215-222='s2.a[0]'): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (215-227='s2.a[0].a[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 6368: (150-157='s1.a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (170-177='s2.a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (197-204='s1.a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (197-209='s1.a[0].a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (215-222='s2.a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (215-227='s2.a[0].a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (190-230): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
