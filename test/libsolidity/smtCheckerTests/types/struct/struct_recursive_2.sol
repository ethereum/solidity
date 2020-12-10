pragma experimental SMTChecker;

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
// ----
// Warning 8115: (81-85): Assertion checker does not yet support the type of this variable.
// Warning 8115: (88-92): Assertion checker does not yet support the type of this variable.
// Warning 7650: (131-135): Assertion checker does not yet support this expression.
// Warning 8364: (131-133): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (139-143): Assertion checker does not yet support this expression.
// Warning 8364: (139-141): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (155-159): Assertion checker does not yet support this expression.
// Warning 8364: (155-157): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (170-174): Assertion checker does not yet support this expression.
// Warning 8364: (170-172): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (193-202): Assertion checker does not yet support this expression.
// Warning 7650: (193-197): Assertion checker does not yet support this expression.
// Warning 8364: (193-195): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (193-200): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (206-215): Assertion checker does not yet support this expression.
// Warning 7650: (206-210): Assertion checker does not yet support this expression.
// Warning 8364: (206-208): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (206-213): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (246-250): Assertion checker does not yet support this expression.
// Warning 8364: (246-248): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (246-250): Assertion checker does not support recursive structs.
// Warning 7650: (259-263): Assertion checker does not yet support this expression.
// Warning 8364: (259-261): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (259-263): Assertion checker does not support recursive structs.
// Warning 7650: (272-276): Assertion checker does not yet support this expression.
// Warning 8364: (272-274): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (272-283): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (272-276): Assertion checker does not support recursive structs.
// Warning 7650: (287-291): Assertion checker does not yet support this expression.
// Warning 8364: (287-289): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (287-298): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (287-291): Assertion checker does not support recursive structs.
// Warning 7650: (302-311): Assertion checker does not yet support this expression.
// Warning 7650: (302-306): Assertion checker does not yet support this expression.
// Warning 8364: (302-304): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (302-309): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (302-311): Assertion checker does not support recursive structs.
// Warning 7650: (320-329): Assertion checker does not yet support this expression.
// Warning 7650: (320-324): Assertion checker does not yet support this expression.
// Warning 8364: (320-322): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (320-327): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (320-329): Assertion checker does not support recursive structs.
// Warning 6328: (124-144): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (148-182): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (186-216): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 8115: (81-85): Assertion checker does not yet support the type of this variable.
// Warning 8115: (88-92): Assertion checker does not yet support the type of this variable.
// Warning 7650: (131-135): Assertion checker does not yet support this expression.
// Warning 8364: (131-133): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (139-143): Assertion checker does not yet support this expression.
// Warning 8364: (139-141): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (155-159): Assertion checker does not yet support this expression.
// Warning 8364: (155-157): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (170-174): Assertion checker does not yet support this expression.
// Warning 8364: (170-172): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (193-202): Assertion checker does not yet support this expression.
// Warning 7650: (193-197): Assertion checker does not yet support this expression.
// Warning 8364: (193-195): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (193-200): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (206-215): Assertion checker does not yet support this expression.
// Warning 7650: (206-210): Assertion checker does not yet support this expression.
// Warning 8364: (206-208): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (206-213): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (246-250): Assertion checker does not yet support this expression.
// Warning 8364: (246-248): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (246-250): Assertion checker does not support recursive structs.
// Warning 7650: (259-263): Assertion checker does not yet support this expression.
// Warning 8364: (259-261): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (259-263): Assertion checker does not support recursive structs.
// Warning 7650: (272-276): Assertion checker does not yet support this expression.
// Warning 8364: (272-274): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (272-283): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (272-276): Assertion checker does not support recursive structs.
// Warning 7650: (287-291): Assertion checker does not yet support this expression.
// Warning 8364: (287-289): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (287-298): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (287-291): Assertion checker does not support recursive structs.
// Warning 7650: (302-311): Assertion checker does not yet support this expression.
// Warning 7650: (302-306): Assertion checker does not yet support this expression.
// Warning 8364: (302-304): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (302-309): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (302-311): Assertion checker does not support recursive structs.
// Warning 7650: (320-329): Assertion checker does not yet support this expression.
// Warning 7650: (320-324): Assertion checker does not yet support this expression.
// Warning 8364: (320-322): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (320-327): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (320-329): Assertion checker does not support recursive structs.
