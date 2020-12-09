pragma experimental SMTChecker;

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
// ----
// Warning 8115: (81-85): Assertion checker does not yet support the type of this variable.
// Warning 8115: (88-92): Assertion checker does not yet support the type of this variable.
// Warning 8115: (135-147): Assertion checker does not yet support the type of this variable.
// Warning 8115: (166-178): Assertion checker does not yet support the type of this variable.
// Warning 8364: (155-157): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (160-162): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (150-162): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 8364: (186-188): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (191-193): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (181-193): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 7650: (204-208): Assertion checker does not yet support this expression.
// Warning 8364: (204-206): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 7650: (212-216): Assertion checker does not yet support this expression.
// Warning 8364: (212-214): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (220-224): Assertion checker does not yet support this expression.
// Warning 8364: (220-222): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 7650: (228-232): Assertion checker does not yet support this expression.
// Warning 8364: (228-230): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (244-248): Assertion checker does not yet support this expression.
// Warning 8364: (244-246): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 7650: (252-256): Assertion checker does not yet support this expression.
// Warning 8364: (252-254): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (260-264): Assertion checker does not yet support this expression.
// Warning 8364: (260-262): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 7650: (268-272): Assertion checker does not yet support this expression.
// Warning 8364: (268-270): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (277-281): Assertion checker does not yet support this expression.
// Warning 8364: (277-279): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 4375: (277-281): Assertion checker does not support recursive structs.
// Warning 7650: (366-370): Assertion checker does not yet support this expression.
// Warning 8364: (366-368): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (380-384): Assertion checker does not yet support this expression.
// Warning 8364: (380-382): Assertion checker does not yet implement type struct C.S storage ref
// Warning 6328: (197-233): CHC: Assertion violation happens here.\nCounterexample:\n\nb1 = false\nb2 = false\n\n\nTransaction trace:\nconstructor()\nf(false, false)
// Warning 6328: (237-273): CHC: Assertion violation happens here.\nCounterexample:\n\nb1 = false\nb2 = false\n\n\nTransaction trace:\nconstructor()\nf(false, false)
// Warning 6328: (359-391): CHC: Assertion violation happens here.\nCounterexample:\n\nb1 = false\nb2 = false\n\n\nTransaction trace:\nconstructor()\nf(false, false)
// Warning 8115: (81-85): Assertion checker does not yet support the type of this variable.
// Warning 8115: (88-92): Assertion checker does not yet support the type of this variable.
// Warning 8115: (135-147): Assertion checker does not yet support the type of this variable.
// Warning 8115: (166-178): Assertion checker does not yet support the type of this variable.
// Warning 8364: (155-157): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (160-162): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (150-162): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 8364: (186-188): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (191-193): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (181-193): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 7650: (204-208): Assertion checker does not yet support this expression.
// Warning 8364: (204-206): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 7650: (212-216): Assertion checker does not yet support this expression.
// Warning 8364: (212-214): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (220-224): Assertion checker does not yet support this expression.
// Warning 8364: (220-222): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 7650: (228-232): Assertion checker does not yet support this expression.
// Warning 8364: (228-230): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (244-248): Assertion checker does not yet support this expression.
// Warning 8364: (244-246): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 7650: (252-256): Assertion checker does not yet support this expression.
// Warning 8364: (252-254): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (260-264): Assertion checker does not yet support this expression.
// Warning 8364: (260-262): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 7650: (268-272): Assertion checker does not yet support this expression.
// Warning 8364: (268-270): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (277-281): Assertion checker does not yet support this expression.
// Warning 8364: (277-279): Assertion checker does not yet implement type struct C.S storage pointer
// Warning 4375: (277-281): Assertion checker does not support recursive structs.
// Warning 7650: (366-370): Assertion checker does not yet support this expression.
// Warning 8364: (366-368): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (380-384): Assertion checker does not yet support this expression.
// Warning 8364: (380-382): Assertion checker does not yet implement type struct C.S storage ref
