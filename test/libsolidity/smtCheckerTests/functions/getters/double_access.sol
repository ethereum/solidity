pragma experimental SMTChecker;

contract C {
	struct S {
		uint u;
	}

	S public s;

	function f() public view {
		uint u = this.s();
		uint v = this.s();
		assert(u == s.u); // should hold
		assert(u == v); // should hold
		assert(u == 1); // should fail
	}
}
// ----
// Warning 6328: (226-240): CHC: Assertion violation happens here.\nCounterexample:\ns = {u: 0}\n\n\n\nTransaction trace:\nconstructor()\nState: s = {u: 0}\nf()
