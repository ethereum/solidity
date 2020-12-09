pragma experimental SMTChecker;
pragma abicoder v2;

contract C {
	struct T {
		uint t;
	}
	struct S {
		uint x;
		T t;
		bool b;
		uint[] a;
	}

	S public s;

	function f() public view {
		uint y;
		bool c;
		T memory t;
		(y,t,c) = this.s();
		assert(y == s.x); // this should hold
		assert(c == s.b); // this should hold
		assert(t.t == s.t.t); // this should hold
		assert(c == true); // this should fail
	}
}
// ----
// Warning 6328: (370-387): CHC: Assertion violation happens here.\nCounterexample:\ns = {x: 0, t: {t: 0}, b: false, a: []}\n\n\n\nTransaction trace:\nconstructor()\nState: s = {x: 0, t: {t: 0}, b: false, a: []}\nf()
