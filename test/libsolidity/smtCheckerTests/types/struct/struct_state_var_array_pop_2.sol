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
// ----
// Warning 2529: (133-142): CHC: Empty array "pop" happens here.\nCounterexample:\ns = {x: 38, a: []}\n_x = 38\n\n\nTransaction trace:\nconstructor()\nState: s = {x: 0, a: []}\nf(38)
// Warning 6328: (189-213): CHC: Assertion violation happens here.\nCounterexample:\ns = {x: 115792089237316195423570985008687907853269984665640564039457584007913129639897, a: [115792089237316195423570985008687907853269984665640564039457584007913129639897, 0]}\n_x = 115792089237316195423570985008687907853269984665640564039457584007913129639897\n\n\nTransaction trace:\nconstructor()\nState: s = {x: 0, a: []}\nf(115792089237316195423570985008687907853269984665640564039457584007913129639897)
