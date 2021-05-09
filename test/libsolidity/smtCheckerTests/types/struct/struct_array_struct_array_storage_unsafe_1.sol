contract C {
	struct T {
		uint y;
		uint[] a;
	}
	struct S {
		uint x;
		T t;
		uint[] a;
		T[] ts;
	}
	S s1;
	function f() public {
		s1.x = 2;
		assert(s1.x != 2);
		s1.t.y = 3;
		assert(s1.t.y != 3);
		s1.a.push();
		s1.a.push();
		s1.a.push();
		s1.a[2] = 4;
		assert(s1.a[2] != 4);
		s1.ts.push();
		s1.ts.push();
		s1.ts.push();
		s1.ts.push();
		s1.ts.push();
		s1.ts.push();
		s1.ts[3].y = 5;
		assert(s1.ts[3].y != 5);
		s1.ts[4].a.push();
		s1.ts[4].a.push();
		s1.ts[4].a.push();
		s1.ts[4].a.push();
		s1.ts[4].a.push();
		s1.ts[4].a.push();
		s1.ts[4].a[5] = 6;
		assert(s1.ts[4].a[5] != 6);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (148-165): CHC: Assertion violation happens here.
// Warning 6328: (183-202): CHC: Assertion violation happens here.\nCounterexample:\ns1 = {x: 2, t: {y: 3, a: []}, a: [], ts: []}\n\nTransaction trace:\nC.constructor()\nState: s1 = {x: 0, t: {y: 0, a: []}, a: [], ts: []}\nC.f()
// Warning 6328: (266-286): CHC: Assertion violation happens here.
// Warning 6328: (404-427): CHC: Assertion violation happens here.\nCounterexample:\ns1 = {x: 2, t: {y: 3, a: []}, a: [0, 0, 4], ts: [{y: 0, a: []}, {y: 0, a: []}, {y: 0, a: []}, {y: 5, a: []}, {y: 0, a: []}, {y: 0, a: []}]}\n\nTransaction trace:\nC.constructor()\nState: s1 = {x: 0, t: {y: 0, a: []}, a: [], ts: []}\nC.f()
// Warning 6328: (578-604): CHC: Assertion violation happens here.\nCounterexample:\ns1 = {x: 2, t: {y: 3, a: []}, a: [0, 0, 4], ts: [{y: 0, a: []}, {y: 0, a: []}, {y: 0, a: []}, {y: 5, a: []}, {y: 0, a: [0, 0, 0, 0, 0, 6]}, {y: 0, a: []}]}\n\nTransaction trace:\nC.constructor()\nState: s1 = {x: 0, t: {y: 0, a: []}, a: [], ts: []}\nC.f()
