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
	function f() public pure {
		S memory s1;
		s1.x = 2;
		assert(s1.x == 2);
		s1.t.y = 3;
		assert(s1.t.y == 3);
		s1.a = new uint[](3);
		s1.a[2] = 4;
		assert(s1.a[2] == 4);
		s1.ts = new T[](6);
		s1.ts[3].y = 5;
		assert(s1.ts[3].y == 5);
		s1.ts[4].a = new uint[](6);
		s1.ts[4].a[5] = 6;
		assert(s1.ts[4].a[5] == 6);
	}
}
// ====
// SMTEngine: all
// ----
