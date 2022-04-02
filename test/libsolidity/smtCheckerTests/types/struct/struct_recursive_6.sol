contract C {
	struct S {
		uint x;
		S[] a;
	}
	S s1;
	S s2;
	function f() public {
		s1.x = 10;
		++s1.x;
		s1.x++;
		s2.x = 20;
		--s2.x;
		s2.x--;
		assert(s1.x == s2.x + 6);
		assert(s1.a.length == s2.a.length);
		delete s1;
		assert(s1.x == 0);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 8115: (48-52='S s1'): Assertion checker does not yet support the type of this variable.
// Warning 8115: (55-59='S s2'): Assertion checker does not yet support the type of this variable.
// Warning 7650: (86-90='s1.x'): Assertion checker does not yet support this expression.
// Warning 8364: (86-88='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (86-90='s1.x'): Assertion checker does not support recursive structs.
// Warning 7650: (101-105='s1.x'): Assertion checker does not yet support this expression.
// Warning 8364: (101-103='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (101-105='s1.x'): Assertion checker does not support recursive structs.
// Warning 7650: (109-113='s1.x'): Assertion checker does not yet support this expression.
// Warning 8364: (109-111='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (109-113='s1.x'): Assertion checker does not support recursive structs.
// Warning 7650: (119-123='s2.x'): Assertion checker does not yet support this expression.
// Warning 8364: (119-121='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (119-123='s2.x'): Assertion checker does not support recursive structs.
// Warning 7650: (134-138='s2.x'): Assertion checker does not yet support this expression.
// Warning 8364: (134-136='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (134-138='s2.x'): Assertion checker does not support recursive structs.
// Warning 7650: (142-146='s2.x'): Assertion checker does not yet support this expression.
// Warning 8364: (142-144='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (142-146='s2.x'): Assertion checker does not support recursive structs.
// Warning 7650: (159-163='s1.x'): Assertion checker does not yet support this expression.
// Warning 8364: (159-161='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (167-171='s2.x'): Assertion checker does not yet support this expression.
// Warning 8364: (167-169='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (187-191='s1.a'): Assertion checker does not yet support this expression.
// Warning 8364: (187-189='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (202-206='s2.a'): Assertion checker does not yet support this expression.
// Warning 8364: (202-204='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (225-227='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (238-242='s1.x'): Assertion checker does not yet support this expression.
// Warning 8364: (238-240='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4984: (99-105='++s1.x'): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 4984: (109-115='s1.x++'): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 3944: (132-138='--s2.x'): CHC: Underflow (resulting value less than 0) happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 3944: (142-148='s2.x--'): CHC: Underflow (resulting value less than 0) happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 4984: (167-175='s2.x + 6'): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (152-176='assert(s1.x == s2.x + 6)'): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (180-214): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (231-248='assert(s1.x == 0)'): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
