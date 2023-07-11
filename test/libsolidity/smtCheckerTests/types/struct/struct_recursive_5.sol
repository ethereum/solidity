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
// Warning 8364: (93-102): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (120-133): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (137-148): Assertion checker does not yet support this expression.
// Warning 8364: (137-146): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (137-155): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (137-148): Assertion checker does not support recursive structs.
// Warning 7650: (166-177): Assertion checker does not yet support this expression.
// Warning 8364: (166-175): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (188-195): Assertion checker does not yet support this expression.
// Warning 8364: (188-193): Assertion checker does not yet implement type struct C.S storage ref
// Warning 6328: (159-203): CHC: Assertion violation happens here.
// Info 1391: CHC: 6 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
