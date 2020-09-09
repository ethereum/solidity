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
		S memory s1;
		s1.x = 2;
		assert(s1.x == s2.x);
		s1.t.y = 3;
		assert(s1.t.y == s2.t.y);
		s1.a[2] = 4;
		assert(s1.a[2] == s2.a[2]);
		s1.ts[3].y = 5;
		assert(s1.ts[3].y == s2.ts[3].y);
		s1.ts[4].a[5] = 6;
		assert(s1.ts[4].a[5] == s2.ts[4].a[5]);
	}
}
// ----
// Warning 6328: (239-259): Assertion violation happens here.
// Warning 6328: (277-301): Assertion violation happens here.
// Warning 6328: (320-346): Assertion violation happens here.
// Warning 6328: (368-400): Assertion violation happens here.
// Warning 6328: (425-463): Assertion violation happens here.
