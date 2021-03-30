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
		require(s1.a.length > 0);
		require(s2.a.length > 0);
		assert(s1.a[0].x == s2.a[0].x);
		require(s1.a[0].a.length > 0);
		require(s2.a[0].a.length > 0);
		assert(s1.a[0].a.length == s2.a[0].a.length);
		// Disabled because of Spacer killed.
		//require(s1.a[0].a[0].a.length > 0);
		//require(s2.a[0].a[0].a.length > 0);
		//assert(s1.a[0].a[0].x == s2.a[0].a[0].x);
	}
	function g() public {
		s1.x = 42;
		s2.x = 42;
		s1.a.push();
		s2.a.push();
		s1.a[0].x = 43;
		s2.a[0].x = 43;
		s1.a[0].a.push();
		s2.a[0].a.push();
		s1.a[0].a[0].x = 44;
		s2.a[0].a[0].x = 44;
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
// Warning 7650: (194-198): Assertion checker does not yet support this expression.
// Warning 8364: (194-196): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (222-226): Assertion checker does not yet support this expression.
// Warning 8364: (222-224): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (249-258): Assertion checker does not yet support this expression.
// Warning 7650: (249-253): Assertion checker does not yet support this expression.
// Warning 8364: (249-251): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (249-256): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (262-271): Assertion checker does not yet support this expression.
// Warning 7650: (262-266): Assertion checker does not yet support this expression.
// Warning 8364: (262-264): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (262-269): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (284-293): Assertion checker does not yet support this expression.
// Warning 7650: (284-288): Assertion checker does not yet support this expression.
// Warning 8364: (284-286): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (284-291): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (317-326): Assertion checker does not yet support this expression.
// Warning 7650: (317-321): Assertion checker does not yet support this expression.
// Warning 8364: (317-319): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (317-324): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (349-358): Assertion checker does not yet support this expression.
// Warning 7650: (349-353): Assertion checker does not yet support this expression.
// Warning 8364: (349-351): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (349-356): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (369-378): Assertion checker does not yet support this expression.
// Warning 7650: (369-373): Assertion checker does not yet support this expression.
// Warning 8364: (369-371): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (369-376): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (582-586): Assertion checker does not yet support this expression.
// Warning 8364: (582-584): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (582-586): Assertion checker does not support recursive structs.
// Warning 7650: (595-599): Assertion checker does not yet support this expression.
// Warning 8364: (595-597): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (595-599): Assertion checker does not support recursive structs.
// Warning 7650: (608-612): Assertion checker does not yet support this expression.
// Warning 8364: (608-610): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (608-619): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (608-612): Assertion checker does not support recursive structs.
// Warning 7650: (623-627): Assertion checker does not yet support this expression.
// Warning 8364: (623-625): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (623-634): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (623-627): Assertion checker does not support recursive structs.
// Warning 7650: (638-647): Assertion checker does not yet support this expression.
// Warning 7650: (638-642): Assertion checker does not yet support this expression.
// Warning 8364: (638-640): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (638-645): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (638-647): Assertion checker does not support recursive structs.
// Warning 7650: (656-665): Assertion checker does not yet support this expression.
// Warning 7650: (656-660): Assertion checker does not yet support this expression.
// Warning 8364: (656-658): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (656-663): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (656-665): Assertion checker does not support recursive structs.
// Warning 7650: (674-683): Assertion checker does not yet support this expression.
// Warning 7650: (674-678): Assertion checker does not yet support this expression.
// Warning 8364: (674-676): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (674-681): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (674-690): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (674-683): Assertion checker does not support recursive structs.
// Warning 7650: (694-703): Assertion checker does not yet support this expression.
// Warning 7650: (694-698): Assertion checker does not yet support this expression.
// Warning 8364: (694-696): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (694-701): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (694-710): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (694-703): Assertion checker does not support recursive structs.
// Warning 7650: (714-728): Assertion checker does not yet support this expression.
// Warning 7650: (714-723): Assertion checker does not yet support this expression.
// Warning 7650: (714-718): Assertion checker does not yet support this expression.
// Warning 8364: (714-716): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (714-721): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (714-726): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (714-728): Assertion checker does not support recursive structs.
// Warning 7650: (737-751): Assertion checker does not yet support this expression.
// Warning 7650: (737-746): Assertion checker does not yet support this expression.
// Warning 7650: (737-741): Assertion checker does not yet support this expression.
// Warning 8364: (737-739): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (737-744): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (737-749): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (737-751): Assertion checker does not support recursive structs.
// Warning 6328: (124-144): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (148-182): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (249-256): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (262-269): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (242-272): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (284-291): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (317-324): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (349-356): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (369-376): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (342-386): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (638-645): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (656-663): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (674-681): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (694-701): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (714-721): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (714-726): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (737-744): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (737-749): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
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
// Warning 7650: (194-198): Assertion checker does not yet support this expression.
// Warning 8364: (194-196): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (222-226): Assertion checker does not yet support this expression.
// Warning 8364: (222-224): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (249-258): Assertion checker does not yet support this expression.
// Warning 7650: (249-253): Assertion checker does not yet support this expression.
// Warning 8364: (249-251): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (249-256): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (262-271): Assertion checker does not yet support this expression.
// Warning 7650: (262-266): Assertion checker does not yet support this expression.
// Warning 8364: (262-264): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (262-269): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (284-293): Assertion checker does not yet support this expression.
// Warning 7650: (284-288): Assertion checker does not yet support this expression.
// Warning 8364: (284-286): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (284-291): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (317-326): Assertion checker does not yet support this expression.
// Warning 7650: (317-321): Assertion checker does not yet support this expression.
// Warning 8364: (317-319): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (317-324): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (349-358): Assertion checker does not yet support this expression.
// Warning 7650: (349-353): Assertion checker does not yet support this expression.
// Warning 8364: (349-351): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (349-356): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (369-378): Assertion checker does not yet support this expression.
// Warning 7650: (369-373): Assertion checker does not yet support this expression.
// Warning 8364: (369-371): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (369-376): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (582-586): Assertion checker does not yet support this expression.
// Warning 8364: (582-584): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (582-586): Assertion checker does not support recursive structs.
// Warning 7650: (595-599): Assertion checker does not yet support this expression.
// Warning 8364: (595-597): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (595-599): Assertion checker does not support recursive structs.
// Warning 7650: (608-612): Assertion checker does not yet support this expression.
// Warning 8364: (608-610): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (608-619): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (608-612): Assertion checker does not support recursive structs.
// Warning 7650: (623-627): Assertion checker does not yet support this expression.
// Warning 8364: (623-625): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (623-634): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (623-627): Assertion checker does not support recursive structs.
// Warning 7650: (638-647): Assertion checker does not yet support this expression.
// Warning 7650: (638-642): Assertion checker does not yet support this expression.
// Warning 8364: (638-640): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (638-645): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (638-647): Assertion checker does not support recursive structs.
// Warning 7650: (656-665): Assertion checker does not yet support this expression.
// Warning 7650: (656-660): Assertion checker does not yet support this expression.
// Warning 8364: (656-658): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (656-663): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (656-665): Assertion checker does not support recursive structs.
// Warning 7650: (674-683): Assertion checker does not yet support this expression.
// Warning 7650: (674-678): Assertion checker does not yet support this expression.
// Warning 8364: (674-676): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (674-681): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (674-690): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (674-683): Assertion checker does not support recursive structs.
// Warning 7650: (694-703): Assertion checker does not yet support this expression.
// Warning 7650: (694-698): Assertion checker does not yet support this expression.
// Warning 8364: (694-696): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (694-701): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (694-710): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (694-703): Assertion checker does not support recursive structs.
// Warning 7650: (714-728): Assertion checker does not yet support this expression.
// Warning 7650: (714-723): Assertion checker does not yet support this expression.
// Warning 7650: (714-718): Assertion checker does not yet support this expression.
// Warning 8364: (714-716): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (714-721): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (714-726): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (714-728): Assertion checker does not support recursive structs.
// Warning 7650: (737-751): Assertion checker does not yet support this expression.
// Warning 7650: (737-746): Assertion checker does not yet support this expression.
// Warning 7650: (737-741): Assertion checker does not yet support this expression.
// Warning 8364: (737-739): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (737-744): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (737-749): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (737-751): Assertion checker does not support recursive structs.
