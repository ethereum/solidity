contract D {
	uint public x;
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
		assert(s.d.x() == 0); // should hold, but fails because we havoc the state
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTTargets: assert
// SMTIgnoreCex: yes
// ----
// Warning 6328: (179-199): CHC: Assertion violation happens here.
