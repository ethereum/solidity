pragma experimental SMTChecker;

contract C {
	struct S {
		uint x;
		S[] a;
	}
	S[] sa;
	S[][] sa2;
	function f() public {
		sa.push();
		sa2.push();
		sa2[0].push();
		sa2[0][0].a.push();
		assert(sa2[0][0].a.length == sa[0].a.length);
	}
}
// ----
// Warning 8364: (126-135): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (153-166): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (170-181): Assertion checker does not yet support this expression.
// Warning 8364: (170-179): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (170-188): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (170-181): Assertion checker does not support recursive structs.
// Warning 7650: (199-210): Assertion checker does not yet support this expression.
// Warning 8364: (199-208): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (221-228): Assertion checker does not yet support this expression.
// Warning 8364: (221-226): Assertion checker does not yet implement type struct C.S storage ref
// Warning 6328: (192-236): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 8364: (126-135): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (153-166): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (170-181): Assertion checker does not yet support this expression.
// Warning 8364: (170-179): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (170-188): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (170-181): Assertion checker does not support recursive structs.
// Warning 7650: (199-210): Assertion checker does not yet support this expression.
// Warning 8364: (199-208): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (221-228): Assertion checker does not yet support this expression.
// Warning 8364: (221-226): Assertion checker does not yet implement type struct C.S storage ref
