pragma abicoder               v2;

contract C {
	struct S {
		uint x;
		uint[] a;
	}
	function f(S memory s1, S memory s2, bool b) public pure {
		S memory s3 = b ? s1 : s2;
		assert(s3.x == s1.x);
		assert(s3.x == s2.x);
		// This is safe.
		assert(s3.x == s1.x || s3.x == s2.x);
		// This fails as false positive because of lack of support to aliasing.
		s3.x = 42;
		assert(s3.x == s1.x || s3.x == s2.x);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (176-196): CHC: Assertion violation happens here.
// Warning 6328: (200-220): CHC: Assertion violation happens here.
// Warning 6328: (370-406): CHC: Assertion violation happens here.
