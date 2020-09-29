pragma experimental SMTChecker;
pragma experimental ABIEncoderV2;

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
		assert(s1.length == 3);
		s1[0].x = 2;
		assert(s1[0].x == s2.x);
		s1[1].t.y = 3;
		assert(s1[1].t.y == s2.t.y);
		s1[2].a[2] = 4;
		assert(s1[2].a[2] == s2.a[2]);
		s1[0].ts[3].y = 5;
		assert(s1[0].ts[3].y == s2.ts[3].y);
		s1[1].ts[4].a[5] = 6;
		assert(s1[1].ts[4].a[5] == s2.ts[4].a[5]);
	}
}
// ----
// Warning 6328: (283-306): CHC: Assertion violation happens here.
// Warning 6328: (327-354): CHC: Assertion violation happens here.
// Warning 6328: (376-405): CHC: Assertion violation happens here.
// Warning 6328: (430-465): CHC: Assertion violation happens here.
// Warning 6328: (493-534): CHC: Assertion violation happens here.
