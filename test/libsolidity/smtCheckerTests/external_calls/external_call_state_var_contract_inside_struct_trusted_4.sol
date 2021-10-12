contract D {
	uint public x;
	function setD(uint _x) public { x = _x; }
}

contract C {
	struct S {
		D d;
	}
	struct T {
		S s;
	}
	T t;
	constructor() {
		t.s.d = new D();
		assert(t.s.d.x() == 0); // should hold
	}
	function f() public view {
		assert(t.s.d.x() == 0); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTTargets: assert
// ----
// Warning 6328: (248-270): CHC: Assertion violation happens here.\nCounterexample:\nt = {s: {d: 20819}}\n\nTransaction trace:\nC.constructor()\nState: t = {s: {d: 20819}}\nC.f()
