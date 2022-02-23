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
// Warning 8115: (48-52): Assertion checker does not yet support the type of this variable.
// Warning 8115: (55-59): Assertion checker does not yet support the type of this variable.
// Warning 8115: (102-114): Assertion checker does not yet support the type of this variable.
// Warning 8115: (133-145): Assertion checker does not yet support the type of this variable.
// Warning 8364: (122-124): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (127-129): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (117-129): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 8364: (153-155): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (158-160): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (148-160): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 7650: (171-175): Assertion checker does not yet support this expression.
// Warning 8364: (171-173): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 7650: (179-183): Assertion checker does not yet support this expression.
// Warning 8364: (179-181): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (187-191): Assertion checker does not yet support this expression.
// Warning 8364: (187-189): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 7650: (195-199): Assertion checker does not yet support this expression.
// Warning 8364: (195-197): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (211-215): Assertion checker does not yet support this expression.
// Warning 8364: (211-213): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 7650: (219-223): Assertion checker does not yet support this expression.
// Warning 8364: (219-221): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (227-231): Assertion checker does not yet support this expression.
// Warning 8364: (227-229): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 7650: (235-239): Assertion checker does not yet support this expression.
// Warning 8364: (235-237): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (244-248): Assertion checker does not yet support this expression.
// Warning 8364: (244-246): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 4375: (244-248): Assertion checker does not support recursive structs.
// Warning 7650: (333-337): Assertion checker does not yet support this expression.
// Warning 8364: (333-335): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (347-351): Assertion checker does not yet support this expression.
// Warning 8364: (347-349): Assertion checker does not yet implement type struct C.S storage ref
// Warning 6328: (164-200): CHC: Assertion violation happens here.
// Warning 6328: (204-240): CHC: Assertion violation happens here.
// Warning 6328: (326-358): CHC: Assertion violation happens here.
