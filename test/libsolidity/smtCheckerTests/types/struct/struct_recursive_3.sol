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
// Warning 8115: (48-52='S s1'): Assertion checker does not yet support the type of this variable.
// Warning 8115: (55-59='S s2'): Assertion checker does not yet support the type of this variable.
// Warning 7650: (98-102='s1.x'): Assertion checker does not yet support this expression.
// Warning 8364: (98-100='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (106-110='s2.x'): Assertion checker does not yet support this expression.
// Warning 8364: (106-108='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (122-126='s1.a'): Assertion checker does not yet support this expression.
// Warning 8364: (122-124='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (137-141='s2.a'): Assertion checker does not yet support this expression.
// Warning 8364: (137-139='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (161-165='s1.a'): Assertion checker does not yet support this expression.
// Warning 8364: (161-163='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (189-193='s2.a'): Assertion checker does not yet support this expression.
// Warning 8364: (189-191='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (216-225='s1.a[0].x'): Assertion checker does not yet support this expression.
// Warning 7650: (216-220='s1.a'): Assertion checker does not yet support this expression.
// Warning 8364: (216-218='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (216-223='s1.a[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (229-238='s2.a[0].x'): Assertion checker does not yet support this expression.
// Warning 7650: (229-233='s2.a'): Assertion checker does not yet support this expression.
// Warning 8364: (229-231='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (229-236='s2.a[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (251-260='s1.a[0].a'): Assertion checker does not yet support this expression.
// Warning 7650: (251-255='s1.a'): Assertion checker does not yet support this expression.
// Warning 8364: (251-253='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (251-258='s1.a[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (284-293='s2.a[0].a'): Assertion checker does not yet support this expression.
// Warning 7650: (284-288='s2.a'): Assertion checker does not yet support this expression.
// Warning 8364: (284-286='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (284-291='s2.a[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (316-325='s1.a[0].a'): Assertion checker does not yet support this expression.
// Warning 7650: (316-320='s1.a'): Assertion checker does not yet support this expression.
// Warning 8364: (316-318='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (316-323='s1.a[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (336-345='s2.a[0].a'): Assertion checker does not yet support this expression.
// Warning 7650: (336-340='s2.a'): Assertion checker does not yet support this expression.
// Warning 8364: (336-338='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (336-343='s2.a[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (549-553='s1.x'): Assertion checker does not yet support this expression.
// Warning 8364: (549-551='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (549-553='s1.x'): Assertion checker does not support recursive structs.
// Warning 7650: (562-566='s2.x'): Assertion checker does not yet support this expression.
// Warning 8364: (562-564='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (562-566='s2.x'): Assertion checker does not support recursive structs.
// Warning 7650: (575-579='s1.a'): Assertion checker does not yet support this expression.
// Warning 8364: (575-577='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (575-586='s1.a.push()'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (575-579='s1.a'): Assertion checker does not support recursive structs.
// Warning 7650: (590-594='s2.a'): Assertion checker does not yet support this expression.
// Warning 8364: (590-592='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (590-601='s2.a.push()'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (590-594='s2.a'): Assertion checker does not support recursive structs.
// Warning 7650: (605-614='s1.a[0].x'): Assertion checker does not yet support this expression.
// Warning 7650: (605-609='s1.a'): Assertion checker does not yet support this expression.
// Warning 8364: (605-607='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (605-612='s1.a[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (605-614='s1.a[0].x'): Assertion checker does not support recursive structs.
// Warning 7650: (623-632='s2.a[0].x'): Assertion checker does not yet support this expression.
// Warning 7650: (623-627='s2.a'): Assertion checker does not yet support this expression.
// Warning 8364: (623-625='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (623-630='s2.a[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (623-632='s2.a[0].x'): Assertion checker does not support recursive structs.
// Warning 7650: (641-650='s1.a[0].a'): Assertion checker does not yet support this expression.
// Warning 7650: (641-645='s1.a'): Assertion checker does not yet support this expression.
// Warning 8364: (641-643='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (641-648='s1.a[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (641-657='s1.a[0].a.push()'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (641-650='s1.a[0].a'): Assertion checker does not support recursive structs.
// Warning 7650: (661-670='s2.a[0].a'): Assertion checker does not yet support this expression.
// Warning 7650: (661-665='s2.a'): Assertion checker does not yet support this expression.
// Warning 8364: (661-663='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (661-668='s2.a[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (661-677='s2.a[0].a.push()'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (661-670='s2.a[0].a'): Assertion checker does not support recursive structs.
// Warning 7650: (681-695='s1.a[0].a[0].x'): Assertion checker does not yet support this expression.
// Warning 7650: (681-690='s1.a[0].a'): Assertion checker does not yet support this expression.
// Warning 7650: (681-685='s1.a'): Assertion checker does not yet support this expression.
// Warning 8364: (681-683='s1'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (681-688='s1.a[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (681-693='s1.a[0].a[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (681-695='s1.a[0].a[0].x'): Assertion checker does not support recursive structs.
// Warning 7650: (704-718='s2.a[0].a[0].x'): Assertion checker does not yet support this expression.
// Warning 7650: (704-713='s2.a[0].a'): Assertion checker does not yet support this expression.
// Warning 7650: (704-708='s2.a'): Assertion checker does not yet support this expression.
// Warning 8364: (704-706='s2'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (704-711='s2.a[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (704-716='s2.a[0].a[0]'): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (704-718='s2.a[0].a[0].x'): Assertion checker does not support recursive structs.
// Warning 6328: (91-111='assert(s1.x == s2.x)'): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (115-149): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (216-223='s1.a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (229-236='s2.a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (209-239='assert(s1.a[0].x == s2.a[0].x)'): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (251-258='s1.a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (284-291='s2.a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (316-323='s1.a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (336-343='s2.a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (309-353): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (605-612='s1.a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (623-630='s2.a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (641-648='s1.a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (661-668='s2.a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (681-688='s1.a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (681-693='s1.a[0].a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (704-711='s2.a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (704-716='s2.a[0].a[0]'): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
