contract C {
	struct S {
		uint x;
		S[] a;
	}
	S s1;
	S s2;
	function f(bool b1, bool b2) public {
		S storage s3 = b1 ? s1 : s2;
		S storage s4 = b2 ? s1 : s2;
		assert(s3.x == s1.x || s3.x == s2.x);
		assert(s4.x == s1.x || s4.x == s2.x);
		s3.x = 44;
		// Fails as false positive because of lack of support to aliasing.
		assert(s1.x == 44 || s2.x == 44);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 8115: (48-52='S s1'): Assertion checker does not yet support the type of this variable.
// Warning 8115: (55-59='S s2'): Assertion checker does not yet support the type of this variable.
// Warning 8115: (102-114='S storage s3'): Assertion checker does not yet support the type of this variable.
// Warning 8115: (133-145='S storage s4'): Assertion checker does not yet support the type of this variable.
// Warning 8364: (122-124='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (127-129='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (117-129='b1 ? s1 : s2'): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 8364: (153-155='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (158-160='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (148-160='b2 ? s1 : s2'): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 7650: (171-175='s3.x'): Assertion checker does not yet support this expression.
// Warning 8364: (171-173='s3'): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 7650: (179-183='s1.x'): Assertion checker does not yet support this expression.
// Warning 8364: (179-181='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (187-191='s3.x'): Assertion checker does not yet support this expression.
// Warning 8364: (187-189='s3'): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 7650: (195-199='s2.x'): Assertion checker does not yet support this expression.
// Warning 8364: (195-197='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (211-215='s4.x'): Assertion checker does not yet support this expression.
// Warning 8364: (211-213='s4'): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 7650: (219-223='s1.x'): Assertion checker does not yet support this expression.
// Warning 8364: (219-221='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (227-231='s4.x'): Assertion checker does not yet support this expression.
// Warning 8364: (227-229='s4'): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 7650: (235-239='s2.x'): Assertion checker does not yet support this expression.
// Warning 8364: (235-237='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (244-248='s3.x'): Assertion checker does not yet support this expression.
// Warning 8364: (244-246='s3'): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 4375: (244-248='s3.x'): Assertion checker does not support recursive structs.
// Warning 7650: (333-337='s1.x'): Assertion checker does not yet support this expression.
// Warning 8364: (333-335='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (347-351='s2.x'): Assertion checker does not yet support this expression.
// Warning 8364: (347-349='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 6328: (164-200): CHC: Assertion violation happens here.
// Warning 6328: (204-240): CHC: Assertion violation happens here.
// Warning 6328: (326-358='assert(s1.x == 44 || s2.x == 44)'): CHC: Assertion violation happens here.
