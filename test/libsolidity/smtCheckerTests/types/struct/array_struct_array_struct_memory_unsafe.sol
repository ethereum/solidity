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
	function f(S memory s2) public pure {
		S[] memory s1 = new S[](3);
		assert(s1.length == 3); // should hold
		s1[0].x = 2;
		assert(s1[0].x == s2.x); // should fail
		s1[1].t.y = 3;
		assert(s1[1].t.y == s2.t.y); // should fail
		s1[2].a = new uint[](3);
		s1[2].a[2] = 4;
		assert(s1[2].a[2] == s2.a[2]); // should fail
		s1[0].ts = new T[](4);
		s1[0].ts[3].y = 5;
		assert(s1[0].ts[3].y == s2.ts[3].y); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (266-289): CHC: Assertion violation happens here.
// Warning 6328: (325-352): CHC: Assertion violation happens here.
// Warning 6368: (437-444): CHC: Out of bounds access happens here.
// Warning 6328: (416-445): CHC: Assertion violation happens here.
// Warning 6368: (534-542): CHC: Out of bounds access happens here.
// Warning 6328: (510-545): CHC: Assertion violation happens here.
