pragma experimental SMTChecker;

contract C {
	struct S {
		uint x;
		uint[] a;
	}
	S s;
	function f(uint _x) public {
		s.x = _x;
		s.a.pop();
		s.a.push();
		s.a.push();
		s.a[0] = _x;
		assert(s.a[1] == s.a[0]);
		s.a.pop();
		s.a.pop();
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 2529: (133-142): CHC: Empty array "pop" happens here.
// Warning 6328: (189-213): CHC: Assertion violation happens here.
