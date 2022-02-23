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
// Warning 8115: (48-52): Assertion checker does not yet support the type of this variable.
// Warning 8115: (55-59): Assertion checker does not yet support the type of this variable.
// Warning 7650: (86-90): Assertion checker does not yet support this expression.
// Warning 8364: (86-88): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (86-90): Assertion checker does not support recursive structs.
// Warning 7650: (101-105): Assertion checker does not yet support this expression.
// Warning 8364: (101-103): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (101-105): Assertion checker does not support recursive structs.
// Warning 7650: (109-113): Assertion checker does not yet support this expression.
// Warning 8364: (109-111): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (109-113): Assertion checker does not support recursive structs.
// Warning 7650: (119-123): Assertion checker does not yet support this expression.
// Warning 8364: (119-121): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (119-123): Assertion checker does not support recursive structs.
// Warning 7650: (134-138): Assertion checker does not yet support this expression.
// Warning 8364: (134-136): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (134-138): Assertion checker does not support recursive structs.
// Warning 7650: (142-146): Assertion checker does not yet support this expression.
// Warning 8364: (142-144): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (142-146): Assertion checker does not support recursive structs.
// Warning 7650: (159-163): Assertion checker does not yet support this expression.
// Warning 8364: (159-161): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (167-171): Assertion checker does not yet support this expression.
// Warning 8364: (167-169): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (187-191): Assertion checker does not yet support this expression.
// Warning 8364: (187-189): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (202-206): Assertion checker does not yet support this expression.
// Warning 8364: (202-204): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (225-227): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (238-242): Assertion checker does not yet support this expression.
// Warning 8364: (238-240): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4984: (99-105): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 4984: (109-115): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 3944: (132-138): CHC: Underflow (resulting value less than 0) happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 3944: (142-148): CHC: Underflow (resulting value less than 0) happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 4984: (167-175): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (152-176): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (180-214): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (231-248): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
