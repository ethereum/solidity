//pragma abicoder v2;

contract C {
	struct S {
		uint[2] a;
		uint u;
	}

	S public s;

	function f() public view {
		uint u = this.s();
		assert(u == s.u); // should hold
		assert(u == 1); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (175-189): CHC: Assertion violation happens here.\nCounterexample:\ns = {a: [0, 0], u: 0}\nu = 0\n\nTransaction trace:\nC.constructor()\nState: s = {a: [0, 0], u: 0}\nC.f()
