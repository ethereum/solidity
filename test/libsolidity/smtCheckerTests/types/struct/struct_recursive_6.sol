pragma experimental SMTChecker;

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
// ----
// Warning 8115: (81-85): Assertion checker does not yet support the type of this variable.
// Warning 8115: (88-92): Assertion checker does not yet support the type of this variable.
// Warning 7650: (119-123): Assertion checker does not yet support this expression.
// Warning 8364: (119-121): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (119-123): Assertion checker does not support recursive structs.
// Warning 7650: (134-138): Assertion checker does not yet support this expression.
// Warning 8364: (134-136): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (134-138): Assertion checker does not support recursive structs.
// Warning 7650: (142-146): Assertion checker does not yet support this expression.
// Warning 8364: (142-144): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (142-146): Assertion checker does not support recursive structs.
// Warning 7650: (152-156): Assertion checker does not yet support this expression.
// Warning 8364: (152-154): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (152-156): Assertion checker does not support recursive structs.
// Warning 7650: (167-171): Assertion checker does not yet support this expression.
// Warning 8364: (167-169): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (167-171): Assertion checker does not support recursive structs.
// Warning 7650: (175-179): Assertion checker does not yet support this expression.
// Warning 8364: (175-177): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (175-179): Assertion checker does not support recursive structs.
// Warning 7650: (192-196): Assertion checker does not yet support this expression.
// Warning 8364: (192-194): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (200-204): Assertion checker does not yet support this expression.
// Warning 8364: (200-202): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (220-224): Assertion checker does not yet support this expression.
// Warning 8364: (220-222): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (235-239): Assertion checker does not yet support this expression.
// Warning 8364: (235-237): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (258-260): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (271-275): Assertion checker does not yet support this expression.
// Warning 8364: (271-273): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4984: (132-138): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 4984: (142-148): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 3944: (165-171): CHC: Underflow (resulting value less than 0) happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 3944: (175-181): CHC: Underflow (resulting value less than 0) happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 4984: (200-208): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (185-209): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (213-247): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (264-281): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 8115: (81-85): Assertion checker does not yet support the type of this variable.
// Warning 8115: (88-92): Assertion checker does not yet support the type of this variable.
// Warning 7650: (119-123): Assertion checker does not yet support this expression.
// Warning 8364: (119-121): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (119-123): Assertion checker does not support recursive structs.
// Warning 7650: (134-138): Assertion checker does not yet support this expression.
// Warning 8364: (134-136): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (134-138): Assertion checker does not support recursive structs.
// Warning 7650: (142-146): Assertion checker does not yet support this expression.
// Warning 8364: (142-144): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (142-146): Assertion checker does not support recursive structs.
// Warning 7650: (152-156): Assertion checker does not yet support this expression.
// Warning 8364: (152-154): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (152-156): Assertion checker does not support recursive structs.
// Warning 7650: (167-171): Assertion checker does not yet support this expression.
// Warning 8364: (167-169): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (167-171): Assertion checker does not support recursive structs.
// Warning 7650: (175-179): Assertion checker does not yet support this expression.
// Warning 8364: (175-177): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (175-179): Assertion checker does not support recursive structs.
// Warning 7650: (192-196): Assertion checker does not yet support this expression.
// Warning 8364: (192-194): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (200-204): Assertion checker does not yet support this expression.
// Warning 8364: (200-202): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (220-224): Assertion checker does not yet support this expression.
// Warning 8364: (220-222): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (235-239): Assertion checker does not yet support this expression.
// Warning 8364: (235-237): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (258-260): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (271-275): Assertion checker does not yet support this expression.
// Warning 8364: (271-273): Assertion checker does not yet implement type struct C.S storage ref
