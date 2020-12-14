pragma experimental SMTChecker;
pragma abicoder               v2;

contract C {
	struct S {
		uint x;
		uint[] a;
	}
	function f(S memory s1, S memory s2) public pure {
		delete s1;
		s1.x = 100;
		s1.x--;
		--s1.x;
		assert(s1.x == 98);
		assert(s1.x == s2.x);
	}
}
// ----
// Warning 6328: (240-260): CHC: Assertion violation happens here.\nCounterexample:\n\ns1 = {x: 98, a: []}\ns2 = {x: (- 38), a: [6, 6, 6, 6, 6, 6, 6]}\n\n\nTransaction trace:\nconstructor()\nf({x: 0, a: []}, {x: (- 38), a: [6, 6, 6, 6, 6, 6, 6]})
