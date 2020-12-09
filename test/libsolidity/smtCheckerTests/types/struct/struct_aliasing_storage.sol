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
		// Disabled because Spacer 4.8.9 seg fauts.
		//assert(s3.x == s1.x);
		//assert(s3.x == s2.x);
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
// Warning 6328: (402-438): CHC: Assertion violation happens here.\nCounterexample:\ns1 = {x: 0, a: []}, s2 = {x: 0, a: []}\nb = false\n\n\nTransaction trace:\nconstructor()\nState: s1 = {x: 0, a: []}, s2 = {x: 0, a: []}\nf(false)
