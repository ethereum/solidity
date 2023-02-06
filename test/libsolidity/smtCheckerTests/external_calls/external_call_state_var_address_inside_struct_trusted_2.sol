contract D {
	uint public x;
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
		assert(D(s.d).x() == 0); // should hold, but fails because we havoc the state
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTTargets: assert
// ----
// Warning 6328: (197-220): CHC: Assertion violation happens here.\nCounterexample:\ns = {d: 0x5039}\n\nTransaction trace:\nC.constructor()\nState: s = {d: 0x5039}\nC.f()
