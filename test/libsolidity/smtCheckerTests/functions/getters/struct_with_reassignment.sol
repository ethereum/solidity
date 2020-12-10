pragma experimental SMTChecker;

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
// ----
// Warning 6328: (288-305): CHC: Assertion violation happens here.\nCounterexample:\ns = {x: 1, b: false}\n\n\n\nTransaction trace:\nconstructor()\nState: s = {x: 1, b: false}\nf()
// Warning 6328: (410-424): CHC: Assertion violation happens here.\nCounterexample:\ns = {x: 42, b: false}\n\n\n\nTransaction trace:\nconstructor()\nState: s = {x: 1, b: false}\nf()
