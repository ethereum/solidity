contract C {
	struct S {
		uint x;
		bool b;
	}

	S public s;

	constructor() {
		s.x = 1;
		s.b = false;
	}

	function f() public {
		uint x;
		bool b;
		(x,b) = this.s();
		assert(x == s.x); // this should hold
		assert(b == s.b); // this should hold
		assert(b == true); // this should fail
		s.x = 42;
		(uint y, bool c) = this.s();
		assert(c == b); // this should hold
		assert(y == x); // this should fail

	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: no
// ----
// Warning 6328: (255-272): CHC: Assertion violation happens here.\nCounterexample:\ns = {x: 1, b: false}\nx = 1\nb = false\ny = 0\nc = false\n\nTransaction trace:\nC.constructor()\nState: s = {x: 1, b: false}\nC.f()
// Warning 6328: (377-391): CHC: Assertion violation happens here.\nCounterexample:\ns = {x: 42, b: false}\nx = 1\nb = false\ny = 42\nc = false\n\nTransaction trace:\nC.constructor()\nState: s = {x: 1, b: false}\nC.f()
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
