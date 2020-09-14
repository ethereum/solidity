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
		s1[0].x = 2;
		assert(s1[0].x == s2.x);
		s1[1].t.y = 3;
		assert(s1[1].t.y == s2.t.y);
		s1[2].a[2] = 4;
		assert(s1[2].a[2] == s2.a[2]);
		s1[0].ts[3].y = 5;
		assert(s1[0].ts[3].y == s2.ts[3].y);
	}
}
// ----
// Warning 6328: (257-280): Assertion violation happens here.
// Warning 6328: (301-328): Assertion violation happens here.
// Warning 6328: (350-379): Assertion violation happens here.
// Warning 6328: (404-439): Assertion violation happens here.
// Warning 4588: (228-238): Assertion checker does not yet implement this type of function call.
