contract D {
	uint public x;
	function setD(uint _x) public { x = _x; }
}

contract C {
	struct S {
		address d;
	}
	S s;
	constructor() {
		s.d = address(new D());
		assert(D(s.d).x() == 0); // should hold
	}
	function f() public view {
		assert(D(s.d).x() == 0); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTTargets: assert
// ----
// Warning 6328: (240-263): CHC: Assertion violation happens here.\nCounterexample:\ns = {d: 0x5039}\n\nTransaction trace:\nC.constructor()\nState: s = {d: 0x5039}\nC.f()
