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
// ====
// SMTEngine: all
// ----
// Warning 8364: (93-102='sa.push()'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (120-133='sa2[0].push()'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (137-148='sa2[0][0].a'): Assertion checker does not yet support this expression.
// Warning 8364: (137-146='sa2[0][0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (137-155='sa2[0][0].a.push()'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (137-148='sa2[0][0].a'): Assertion checker does not support recursive structs.
// Warning 7650: (166-177='sa2[0][0].a'): Assertion checker does not yet support this expression.
// Warning 8364: (166-175='sa2[0][0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (188-195='sa[0].a'): Assertion checker does not yet support this expression.
// Warning 8364: (188-193='sa[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 6328: (159-203): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
