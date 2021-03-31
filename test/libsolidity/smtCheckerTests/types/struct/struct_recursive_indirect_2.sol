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
// Warning 8115: (82-86): Assertion checker does not yet support the type of this variable.
// Warning 8115: (89-93): Assertion checker does not yet support the type of this variable.
// Warning 7650: (120-124): Assertion checker does not yet support this expression.
// Warning 8364: (120-122): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (120-131): Assertion checker does not yet implement type struct C.T storage ref
// Warning 4375: (120-124): Assertion checker does not support recursive structs.
// Warning 7650: (135-139): Assertion checker does not yet support this expression.
// Warning 8364: (135-137): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (135-146): Assertion checker does not yet implement type struct C.T storage ref
// Warning 4375: (135-139): Assertion checker does not support recursive structs.
// Warning 7650: (150-159): Assertion checker does not yet support this expression.
// Warning 7650: (150-154): Assertion checker does not yet support this expression.
// Warning 8364: (150-152): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (150-157): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (150-166): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (150-159): Assertion checker does not support recursive structs.
// Warning 7650: (170-179): Assertion checker does not yet support this expression.
// Warning 7650: (170-174): Assertion checker does not yet support this expression.
// Warning 8364: (170-172): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (170-177): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (170-186): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (170-179): Assertion checker does not support recursive structs.
// Warning 7650: (197-211): Assertion checker does not yet support this expression.
// Warning 7650: (197-206): Assertion checker does not yet support this expression.
// Warning 7650: (197-201): Assertion checker does not yet support this expression.
// Warning 8364: (197-199): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (197-204): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (197-209): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (215-229): Assertion checker does not yet support this expression.
// Warning 7650: (215-224): Assertion checker does not yet support this expression.
// Warning 7650: (215-219): Assertion checker does not yet support this expression.
// Warning 8364: (215-217): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (215-222): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (215-227): Assertion checker does not yet implement type struct C.S storage ref
// Warning 6368: (150-157): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (170-177): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (197-204): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (197-209): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (215-222): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (215-227): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (190-230): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 8115: (82-86): Assertion checker does not yet support the type of this variable.
// Warning 8115: (89-93): Assertion checker does not yet support the type of this variable.
// Warning 7650: (120-124): Assertion checker does not yet support this expression.
// Warning 8364: (120-122): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (120-131): Assertion checker does not yet implement type struct C.T storage ref
// Warning 4375: (120-124): Assertion checker does not support recursive structs.
// Warning 7650: (135-139): Assertion checker does not yet support this expression.
// Warning 8364: (135-137): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (135-146): Assertion checker does not yet implement type struct C.T storage ref
// Warning 4375: (135-139): Assertion checker does not support recursive structs.
// Warning 7650: (150-159): Assertion checker does not yet support this expression.
// Warning 7650: (150-154): Assertion checker does not yet support this expression.
// Warning 8364: (150-152): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (150-157): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (150-166): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (150-159): Assertion checker does not support recursive structs.
// Warning 7650: (170-179): Assertion checker does not yet support this expression.
// Warning 7650: (170-174): Assertion checker does not yet support this expression.
// Warning 8364: (170-172): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (170-177): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (170-186): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (170-179): Assertion checker does not support recursive structs.
// Warning 7650: (197-211): Assertion checker does not yet support this expression.
// Warning 7650: (197-206): Assertion checker does not yet support this expression.
// Warning 7650: (197-201): Assertion checker does not yet support this expression.
// Warning 8364: (197-199): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (197-204): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (197-209): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (215-229): Assertion checker does not yet support this expression.
// Warning 7650: (215-224): Assertion checker does not yet support this expression.
// Warning 7650: (215-219): Assertion checker does not yet support this expression.
// Warning 8364: (215-217): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (215-222): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (215-227): Assertion checker does not yet implement type struct C.S storage ref
