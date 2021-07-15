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
		S memory s1;
		s1.x = 2;
		assert(s1.x == s2.x); // should fail
		s1.t.y = 3;
		assert(s1.t.y == s2.t.y); // should fail
		s1.a = new uint[](3);
		s1.a[2] = 4;
		assert(s1.a[2] == s2.a[2]); // should fail
		s1.ts = new T[](6);
		s1.ts[3].y = 5;
		assert(s1.ts[3].y == s2.ts[3].y); // should fail
		s1.ts[4].a = new uint[](6);
		s1.ts[4].a[5] = 6;
		require(s2.ts.length > 4);
		require(s2.ts[4].a.length > 6);
		assert(s1.ts[4].a[5] == s2.ts[4].a[5]); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (207-227): CHC: Assertion violation happens here.
// Warning 6328: (260-284): CHC: Assertion violation happens here.
// Warning 6368: (360-367): CHC: Out of bounds access happens here.
// Warning 6328: (342-368): CHC: Assertion violation happens here.
// Warning 6368: (448-456): CHC: Out of bounds access happens here.
// Warning 6328: (427-459): CHC: Assertion violation happens here.
// Warning 6328: (592-630): CHC: Assertion violation happens here.
