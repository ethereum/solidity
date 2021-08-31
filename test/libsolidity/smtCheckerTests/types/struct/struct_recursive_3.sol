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
// ====
// SMTEngine: all
// ----
// Warning 8115: (48-52): Assertion checker does not yet support the type of this variable.
// Warning 8115: (55-59): Assertion checker does not yet support the type of this variable.
// Warning 7650: (98-102): Assertion checker does not yet support this expression.
// Warning 8364: (98-100): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (106-110): Assertion checker does not yet support this expression.
// Warning 8364: (106-108): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (122-126): Assertion checker does not yet support this expression.
// Warning 8364: (122-124): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (137-141): Assertion checker does not yet support this expression.
// Warning 8364: (137-139): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (161-165): Assertion checker does not yet support this expression.
// Warning 8364: (161-163): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (189-193): Assertion checker does not yet support this expression.
// Warning 8364: (189-191): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (216-225): Assertion checker does not yet support this expression.
// Warning 7650: (216-220): Assertion checker does not yet support this expression.
// Warning 8364: (216-218): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (216-223): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (229-238): Assertion checker does not yet support this expression.
// Warning 7650: (229-233): Assertion checker does not yet support this expression.
// Warning 8364: (229-231): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (229-236): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (251-260): Assertion checker does not yet support this expression.
// Warning 7650: (251-255): Assertion checker does not yet support this expression.
// Warning 8364: (251-253): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (251-258): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (284-293): Assertion checker does not yet support this expression.
// Warning 7650: (284-288): Assertion checker does not yet support this expression.
// Warning 8364: (284-286): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (284-291): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (316-325): Assertion checker does not yet support this expression.
// Warning 7650: (316-320): Assertion checker does not yet support this expression.
// Warning 8364: (316-318): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (316-323): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (336-345): Assertion checker does not yet support this expression.
// Warning 7650: (336-340): Assertion checker does not yet support this expression.
// Warning 8364: (336-338): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (336-343): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (549-553): Assertion checker does not yet support this expression.
// Warning 8364: (549-551): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (549-553): Assertion checker does not support recursive structs.
// Warning 7650: (562-566): Assertion checker does not yet support this expression.
// Warning 8364: (562-564): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (562-566): Assertion checker does not support recursive structs.
// Warning 7650: (575-579): Assertion checker does not yet support this expression.
// Warning 8364: (575-577): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (575-586): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (575-579): Assertion checker does not support recursive structs.
// Warning 7650: (590-594): Assertion checker does not yet support this expression.
// Warning 8364: (590-592): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (590-601): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (590-594): Assertion checker does not support recursive structs.
// Warning 7650: (605-614): Assertion checker does not yet support this expression.
// Warning 7650: (605-609): Assertion checker does not yet support this expression.
// Warning 8364: (605-607): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (605-612): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (605-614): Assertion checker does not support recursive structs.
// Warning 7650: (623-632): Assertion checker does not yet support this expression.
// Warning 7650: (623-627): Assertion checker does not yet support this expression.
// Warning 8364: (623-625): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (623-630): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (623-632): Assertion checker does not support recursive structs.
// Warning 7650: (641-650): Assertion checker does not yet support this expression.
// Warning 7650: (641-645): Assertion checker does not yet support this expression.
// Warning 8364: (641-643): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (641-648): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (641-657): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (641-650): Assertion checker does not support recursive structs.
// Warning 7650: (661-670): Assertion checker does not yet support this expression.
// Warning 7650: (661-665): Assertion checker does not yet support this expression.
// Warning 8364: (661-663): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (661-668): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (661-677): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (661-670): Assertion checker does not support recursive structs.
// Warning 7650: (681-695): Assertion checker does not yet support this expression.
// Warning 7650: (681-690): Assertion checker does not yet support this expression.
// Warning 7650: (681-685): Assertion checker does not yet support this expression.
// Warning 8364: (681-683): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (681-688): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (681-693): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (681-695): Assertion checker does not support recursive structs.
// Warning 7650: (704-718): Assertion checker does not yet support this expression.
// Warning 7650: (704-713): Assertion checker does not yet support this expression.
// Warning 7650: (704-708): Assertion checker does not yet support this expression.
// Warning 8364: (704-706): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (704-711): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (704-716): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (704-718): Assertion checker does not support recursive structs.
// Warning 6328: (91-111): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (115-149): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (216-223): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (229-236): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (209-239): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (251-258): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (284-291): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (316-323): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (336-343): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (309-353): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (605-612): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (623-630): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (641-648): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (661-668): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (681-688): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (681-693): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (704-711): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (704-716): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
