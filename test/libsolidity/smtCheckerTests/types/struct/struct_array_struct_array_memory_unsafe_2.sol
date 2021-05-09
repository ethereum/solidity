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
		// Removed because current Spacer seg faults in cex generation.
		//assert(s1.x == s2.x);
		s1.t.y = 3;
		// Removed because current Spacer seg faults in cex generation.
		//assert(s1.t.y == s2.t.y);
		s1.a = new uint[](3);
		s1.a[2] = 4;
		// Removed because current Spacer seg faults in cex generation.
		//assert(s1.a[2] == s2.a[2]);
		s1.ts = new T[](6);
		s1.ts[3].y = 5;
		// Removed because current Spacer seg faults in cex generation.
		//assert(s1.ts[3].y == s2.ts[3].y);
		s1.ts[4].a = new uint[](6);
		s1.ts[4].a[5] = 6;
		require(s2.ts.length > 4);
		require(s2.ts[4].a.length > 6);
		assert(s1.ts[4].a[5] == s2.ts[4].a[5]);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (804-842): CHC: Assertion violation happens here.
