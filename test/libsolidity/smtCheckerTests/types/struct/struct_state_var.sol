pragma experimental SMTChecker;

contract C {
	struct S {
		uint x;
		uint[] a;
	}
	S s;
	function f(uint _x) public {
		s.x = _x;
		s.a[0] = _x;
		assert(s.a[1] == s.a[0]);
	}
}
// ----
// Warning 6328: (148-172): Assertion violation happens here.
