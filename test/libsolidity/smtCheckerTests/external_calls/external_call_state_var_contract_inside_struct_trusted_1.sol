contract D {
	uint public x;
	function setD(uint _x) public { x = _x; }
}

contract C {
	struct S {
		D d;
	}
	S s;
	constructor() {
		s.d = new D();
		assert(s.d.x() == 0); // should hold
	}
	function f() public view {
		assert(s.d.x() == 0); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTTargets: assert
// ----
// Warning 6328: (222-242): CHC: Assertion violation happens here.\nCounterexample:\ns = {d: 20819}\n\nTransaction trace:\nC.constructor()\nState: s = {d: 20819}\nC.f()
