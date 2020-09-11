pragma experimental SMTChecker;

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
	S s1;
	function f() public {
		s1.x = 2;
		assert(s1.x != 2);
		s1.t.y = 3;
		assert(s1.t.y != 3);
		s1.a[2] = 4;
		assert(s1.a[2] != 4);
		s1.ts[3].y = 5;
		assert(s1.ts[3].y != 5);
	}
}
// ----
// Warning 6328: (181-198): Assertion violation happens here.
// Warning 6328: (216-235): Assertion violation happens here.
// Warning 6328: (254-274): Assertion violation happens here.
// Warning 6328: (296-319): Assertion violation happens here.
