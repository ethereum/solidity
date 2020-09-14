pragma experimental SMTChecker;

contract C {
	struct S {
		uint x;
		uint[] a;
	}
	S s1;
	S s2;
	function f(bool b) public {
		S storage s3 = b ? s1 : s2;
		assert(s3.x == s1.x);
		assert(s3.x == s2.x);
		// This is safe.
		assert(s3.x == s1.x || s3.x == s2.x);
		// This fails as false positive because of lack of support to aliasing.
		s3.x = 42;
		assert(s3.x == s1.x || s3.x == s2.x);
	}
	function g(bool b, uint _x) public {
		if (b)
			s1.x = _x;
		else
			s2.x = _x;
	}
}
// ----
// Warning 6328: (158-178): Assertion violation happens here.
// Warning 6328: (182-202): Assertion violation happens here.
// Warning 6328: (352-388): Assertion violation happens here.
