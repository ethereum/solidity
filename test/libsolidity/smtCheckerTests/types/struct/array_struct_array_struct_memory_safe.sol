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
		S[] memory s1 = new S[](3);
		assert(s1.length == 3);
		s1[0].x = 2;
		assert(s1[0].x == 2);
		s1[1].t.y = 3;
		assert(s1[1].t.y == 3);
		s1[2].a = new uint[](3);
		s1[2].a[2] = 4;
		assert(s1[2].a[2] == 4);
		s1[0].ts = new T[](6);
		s1[0].ts[3].y = 5;
		assert(s1[0].ts[3].y == 5);
		s1[1].ts = new T[](6);
		s1[1].ts[4].a = new uint[](6);
		s1[1].ts[4].a[5] = 6;
		assert(s1[1].ts[4].a[5] == 6);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 29 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
