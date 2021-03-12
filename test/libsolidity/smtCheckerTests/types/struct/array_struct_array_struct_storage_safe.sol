pragma abicoder               v2;

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
	S[] s1;
	function f() public {
		s1.push();
		s1.push();
		s1.push();
		s1[0].x = 2;
		assert(s1[0].x == 2);
		s1[1].t.y = 3;
		assert(s1[1].t.y == 3);
		s1[2].a.push();
		s1[2].a.push();
		s1[2].a.push();
		s1[2].a[2] = 4;
		assert(s1[2].a[2] == 4);
		s1[0].ts.push();
		s1[0].ts.push();
		s1[0].ts.push();
		s1[0].ts.push();
		s1[0].ts.push();
		s1[0].ts[3].y = 5;
		assert(s1[0].ts[3].y == 5);
		s1[1].ts.push();
		s1[1].ts.push();
		s1[1].ts.push();
		s1[1].ts.push();
		s1[1].ts.push();
		s1[1].ts[4].a.push();
		s1[1].ts[4].a.push();
		s1[1].ts[4].a.push();
		s1[1].ts[4].a.push();
		s1[1].ts[4].a.push();
		s1[1].ts[4].a.push();
		s1[1].ts[4].a[5] = 6;
		assert(s1[1].ts[4].a[5] == 6);
		s1.pop();
		s1.pop();
		s1.pop();
	}
}
// ====
// SMTEngine: all
// ----
