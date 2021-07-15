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
// ====
// SMTEngine: all
// ----
// Warning 6328: (125-145): CHC: Assertion violation happens here.\nCounterexample:\ns1 = {x: 0, a: []}, s2 = {x: 115792089237316195423570985008687907853269984665640564039457584007913129639897, a: []}\nb = false\ns3 = {x: 115792089237316195423570985008687907853269984665640564039457584007913129639897, a: []}\n\nTransaction trace:\nC.constructor()\nState: s1 = {x: 0, a: []}, s2 = {x: 0, a: []}\nC.g(false, 115792089237316195423570985008687907853269984665640564039457584007913129639897)\nState: s1 = {x: 0, a: []}, s2 = {x: 115792089237316195423570985008687907853269984665640564039457584007913129639897, a: []}\nC.f(false)
// Warning 6328: (149-169): CHC: Assertion violation happens here.\nCounterexample:\ns1 = {x: 0, a: []}, s2 = {x: 7720, a: []}\nb = true\ns3 = {x: 0, a: []}\n\nTransaction trace:\nC.constructor()\nState: s1 = {x: 0, a: []}, s2 = {x: 0, a: []}\nC.g(false, 7720)\nState: s1 = {x: 0, a: []}, s2 = {x: 7720, a: []}\nC.f(true)
// Warning 6328: (319-355): CHC: Assertion violation happens here.\nCounterexample:\ns1 = {x: 0, a: []}, s2 = {x: 0, a: []}\nb = false\ns3 = {x: 42, a: []}\n\nTransaction trace:\nC.constructor()\nState: s1 = {x: 0, a: []}, s2 = {x: 0, a: []}\nC.f(false)
