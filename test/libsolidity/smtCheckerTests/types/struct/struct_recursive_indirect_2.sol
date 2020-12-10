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
	function f() public {
		s1.a.push();
		s2.a.push();
		s1.a[0].a.push();
		s2.a[0].a.push();
		assert(s1.a[0].a[0].x == s2.a[0].a[0].x);
	}
}
// ----
// Warning 8115: (115-119): Assertion checker does not yet support the type of this variable.
// Warning 8115: (122-126): Assertion checker does not yet support the type of this variable.
// Warning 7650: (153-157): Assertion checker does not yet support this expression.
// Warning 8364: (153-155): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (153-164): Assertion checker does not yet implement type struct C.T storage ref
// Warning 4375: (153-157): Assertion checker does not support recursive structs.
// Warning 7650: (168-172): Assertion checker does not yet support this expression.
// Warning 8364: (168-170): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (168-179): Assertion checker does not yet implement type struct C.T storage ref
// Warning 4375: (168-172): Assertion checker does not support recursive structs.
// Warning 7650: (183-192): Assertion checker does not yet support this expression.
// Warning 7650: (183-187): Assertion checker does not yet support this expression.
// Warning 8364: (183-185): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (183-190): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (183-199): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (183-192): Assertion checker does not support recursive structs.
// Warning 7650: (203-212): Assertion checker does not yet support this expression.
// Warning 7650: (203-207): Assertion checker does not yet support this expression.
// Warning 8364: (203-205): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (203-210): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (203-219): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (203-212): Assertion checker does not support recursive structs.
// Warning 7650: (230-244): Assertion checker does not yet support this expression.
// Warning 7650: (230-239): Assertion checker does not yet support this expression.
// Warning 7650: (230-234): Assertion checker does not yet support this expression.
// Warning 8364: (230-232): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (230-237): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (230-242): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (248-262): Assertion checker does not yet support this expression.
// Warning 7650: (248-257): Assertion checker does not yet support this expression.
// Warning 7650: (248-252): Assertion checker does not yet support this expression.
// Warning 8364: (248-250): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (248-255): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (248-260): Assertion checker does not yet implement type struct C.S storage ref
// Warning 6328: (223-263): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 8115: (115-119): Assertion checker does not yet support the type of this variable.
// Warning 8115: (122-126): Assertion checker does not yet support the type of this variable.
// Warning 7650: (153-157): Assertion checker does not yet support this expression.
// Warning 8364: (153-155): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (153-164): Assertion checker does not yet implement type struct C.T storage ref
// Warning 4375: (153-157): Assertion checker does not support recursive structs.
// Warning 7650: (168-172): Assertion checker does not yet support this expression.
// Warning 8364: (168-170): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (168-179): Assertion checker does not yet implement type struct C.T storage ref
// Warning 4375: (168-172): Assertion checker does not support recursive structs.
// Warning 7650: (183-192): Assertion checker does not yet support this expression.
// Warning 7650: (183-187): Assertion checker does not yet support this expression.
// Warning 8364: (183-185): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (183-190): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (183-199): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (183-192): Assertion checker does not support recursive structs.
// Warning 7650: (203-212): Assertion checker does not yet support this expression.
// Warning 7650: (203-207): Assertion checker does not yet support this expression.
// Warning 8364: (203-205): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (203-210): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (203-219): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (203-212): Assertion checker does not support recursive structs.
// Warning 7650: (230-244): Assertion checker does not yet support this expression.
// Warning 7650: (230-239): Assertion checker does not yet support this expression.
// Warning 7650: (230-234): Assertion checker does not yet support this expression.
// Warning 8364: (230-232): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (230-237): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (230-242): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (248-262): Assertion checker does not yet support this expression.
// Warning 7650: (248-257): Assertion checker does not yet support this expression.
// Warning 7650: (248-252): Assertion checker does not yet support this expression.
// Warning 8364: (248-250): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (248-255): Assertion checker does not yet implement type struct C.T storage ref
// Warning 8364: (248-260): Assertion checker does not yet implement type struct C.S storage ref
